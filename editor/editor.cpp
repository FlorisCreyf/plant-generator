/* TreeMaker: 3D tree model editor
 * Copyright (C) 2017  Floris Creyf
 *
 * TreeMaker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TreeMaker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "editor.h"
#include "file_exporter.h"
#include <cmath>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>

using treemaker::Vec3;
using treemaker::Ray;

Editor::Editor(SharedResources *shared, QWidget *parent) :
		OpenGLEditor(shared, parent)
{
 
}

void Editor::configureTree()
{
	tree.setRadius(0, 0.2f);
	tree.setResolution(0, 8);
	tree.setGeneratedPathSize(0, 12);
	tree.setMaxStemDepth(1);
	tree.generateTree();
	
	initializeTree();
	emit selectionChanged(tree, -1);
}

void Editor::exportObject(const char *filename)
{
	FileExporter f;
	f.setVertices(tree.getVertices(), tree.getVertexCount());
	f.setTriangles(tree.getIndices(), tree.getIndexCount());
	f.exportObj(filename);
}

void Editor::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Control:
		ctrl = true;
		break;
	case Qt::Key_Shift:
		shift = true;
		break;
	case Qt::Key_E:
		extrude();
		break;
	case Qt::Key_Delete:
		removePoint();
		break;
	case Qt::Key_A:
		addStem();
		break;
	}
}

void Editor::keyReleaseEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Control:
		ctrl = false;
		break;
	case Qt::Key_Shift:
		shift = false;
		break;
	case Qt::Key_Tab:
		break;
	}
}

bool Editor::event(QEvent *e)
{
	if (e->type() == QEvent::KeyPress) {
		QKeyEvent *k = static_cast<QKeyEvent *>(e);
		keyPressEvent(k);
		return true;
	}
	return QWidget::event(e);
}

void Editor::mousePressEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	midButton = false;
	
	if (event->button() == Qt::RightButton) {
		int prevStem = selectedStem;

		if (selectedStem >= 0)
			selectPoint(p.x(), p.y());
		if (selectedPoint == -1)
			selectStem(p.x(), p.y());
		if (selectedStem != prevStem)
			selectedPoint = -1;

		update();
	} else if (event->button() == Qt::MidButton) {
		midButton = true;
		camera.setStartCoordinates(p.x(), p.y());
		if (ctrl && !shift)
			camera.action = Camera::ZOOM;
		else if (shift && !ctrl)
			camera.action = Camera::PAN;
		else
			camera.action = Camera::ROTATE;
	} else if (event->button() == Qt::LeftButton) {
		if (selectedPoint >= 0 && selectedStem >= 0)
			selectAxis(p.x(), p.y());
	}

	setFocus();
}

void Editor::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MidButton)
		camera.action = Camera::NONE;
	if (event->button() == Qt::LeftButton)
		axis.clearSelected();
}

void Editor::mouseMoveEvent(QMouseEvent *event)
{
	QPoint point = event->pos();

	switch (camera.action) {
	case Camera::ZOOM:
		camera.zoom(point.y());
		break;
	case Camera::ROTATE:
		camera.setCoordinates(point.x(), point.y());
		break;
	case Camera::PAN:
		camera.setPan(point.x(), point.y());
		break;
	default:
		break;
	}

	if (axis.getSelected() != Axis::NONE)
		movePoint(point.x(), point.y());

	update();
}

void Editor::extrude()
{
	if (selectedPoint < 0)
		return;
	
	std::vector<Vec3> path(tree.getPathSize(selectedStem));
	tree.getPath(selectedStem, path.data());
	path.insert(path.begin() + selectedPoint, path[selectedPoint]);
	tree.setPath(selectedStem, path.data(), path.size());
	selectedPoint += 1;
	
	QPoint p = mapFromGlobal(QCursor::pos());
	setClickOffset(p.x(), p.y(), path[selectedPoint]);
	axis.pickCenter();
	setMouseTracking(true);
}

void Editor::removePoint()
{
	size_t size = tree.getPathSize(selectedStem);
	
	if (selectedPoint <= 0 || size <= 2) {
		tree.deleteStem(selectedStem);
		selectedStem = -1;
		emit selectionChanged(tree, selectedStem);
	} else {
		std::vector<Vec3> path(size);
		tree.getPath(selectedStem, path.data());
		path.erase(path.begin() + selectedPoint);
		tree.setPath(selectedStem, path.data(), path.size());
	}
	
	selectedPoint = -1;
	updateLines(selectedStem);
	change();
	update();
}

void Editor::addStem()
{
	if (selectedStem < 0)
		return;
	
	QPoint p = mapFromGlobal(QCursor::pos());
	float t = closestDistance(selectedStem, p.x(), p.y());
	tree.newStem(selectedStem, t);
	change();
	update();
	emit modeChanged();
}

void Editor::selectStem(int x, int y)
{
	Ray ray;
	ray.direction = camera.getRayDirection(x, y);
	ray.origin = camera.getPosition();
	treemaker::Aabb box = tree.getBoundingBox(0);
	unsigned stem = tree.getStemName(0);
	
	for (unsigned i = 0; box.a.x != box.b.x;) {
		
		float t = treemaker::intersectsAABB(ray, box);
		if (t != 0.0f) {
			auto location = tree.getStemLocation(stem);
			selection.start[1] = location.indexStart;
			selection.count[1] = location.indexCount;
			selectedStem = stem;
			updateLines(stem);
			emit selectionChanged(tree, stem);
			return;
		}
		stem = tree.getStemName(++i);
		box = tree.getBoundingBox(stem);
	}

	selectedStem = -1;
	setMouseTracking(false);
	emit selectionChanged(tree, selectedStem);
}

void Editor::selectPoint(int x, int y)
{
	int size = tree.getPathSize(selectedStem);
	std::vector<Vec3> points(size);
	tree.getPath(selectedStem, &points[0]);

	for (int i = 0; i < size; i++) {
		Vec3 point = camera.toScreenSpace(points[i]);
		float sx = std::pow(point.x - x, 2);
		float sy = std::pow(point.y - y, 2);

		if (std::sqrt(sx + sy) < 10) {
			selectedPoint = i;
			return;
		}
	}

	setMouseTracking(false);
	selectedPoint = -1;
}

void Editor::selectAxis(int x, int y)
{
	std::vector<Vec3> path(tree.getPathSize(selectedStem));
	tree.getPath(selectedStem, &path[0]);
	Vec3 c = path[selectedPoint];
	Vec3 o = camera.getPosition();
	Vec3 d = camera.getRayDirection(x, y);
	setClickOffset(x, y, c);
	axis.pickAxis(c, {o, d});
}

float Editor::closestDistance(unsigned stem, int x, int y)
{
	std::vector<Vec3> wsPath(tree.getGeneratedPathSize(stem));
	std::vector<Vec3> ssPath(wsPath.size());
	
	Vec3 point = {static_cast<float>(x), static_cast<float>(y), 0.0f};
	float min = std::numeric_limits<float>::max();
	size_t closestLine = 0;
	float t = 0.0f;  
	
	tree.getGeneratedPath(stem, &wsPath[0]);
	for (size_t i = 0; i < wsPath.size(); i++) {
		ssPath[i] = camera.toScreenSpace(wsPath[i]);
		ssPath[i].z = 0.0f;
	}

	for (size_t i = 0; i < wsPath.size() - 1; i++) {
		Vec3 a = point - ssPath[i];
		Vec3 b = ssPath[i+1] - ssPath[i];
		float dist = treemaker::project(a, b);

		if (dist < 0.0f)
			dist = 0.0f;
		if (dist > 1.0f)
			dist = 1.0f;

		float mag = treemaker::magnitude(ssPath[i] + dist * b - point);

		if (min > mag) {
			t = dist;
			min = mag;
			closestLine = i;
		}
	}

	t *= treemaker::magnitude(wsPath[closestLine] - wsPath[closestLine+1]);
	for (size_t i = 0; i < closestLine; i++)
		t += treemaker::magnitude(wsPath[i + 1] - wsPath[i]);
	
	return t;
}

void Editor::movePoint(int x, int y)
{
	x += clickOffset[0];
	y += clickOffset[1];

	Vec3 direction = camera.getDirection();
	Ray ray = {camera.getPosition(), camera.getRayDirection(x, y)};
	
	if (selectedPoint == 0 && !tree.isLateral(selectedStem)) {
		Vec3 loc = tree.getLocation(selectedStem);
		loc = axis.move(axis.getSelected(), ray, direction, loc);
		tree.setLocation(selectedStem, loc);
	} else if (selectedPoint == 0) {
		float t = closestDistance(tree.getParent(selectedStem), x, y);
		tree.setPosition(selectedStem, t);
	} else {
		Vec3 loc;
		std::vector<Vec3> path(tree.getPathSize(selectedStem));
		tree.getPath(selectedStem, &path[0]);
		loc = path[selectedPoint];
		loc = axis.move(axis.getSelected(), ray, direction, loc);
		path[selectedPoint] = loc;
		tree.setPath(selectedStem, &path[0], path.size());
		emit modeChanged();
	}
	
	updateLines(selectedStem);
	change();
}

void Editor::setClickOffset(int x, int y, Vec3 point)
{
	Vec3 s = camera.toScreenSpace(point);
	clickOffset[0] = s.x - x;
	clickOffset[1] = s.y - y;
}

treemaker::Tree *Editor::getTree()
{
	return &tree;
}

int Editor::getSelectedStem()
{
	return selectedStem;
}
