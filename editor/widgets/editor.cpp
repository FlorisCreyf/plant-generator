/* Plant Genererator
 * Copyright (C) 2016-2018  Floris Creyf
 *
 * Plant Genererator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Genererator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "editor.h"
#include "../commands/extrude_stem.h"
#include "../commands/remove_stem.h"
#include "../geometry/geometry.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <QVBoxLayout>
#include <QMenu>
#include <QScrollArea>
#include <QtOpenGL/QGLFormat>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <boost/archive/text_iarchive.hpp>

using pg::Vec3;
using pg::Mat4;
using pg::Ray;

Editor::Editor(SharedResources *shared, QWidget *parent) :
	QOpenGLWidget(parent),
	mesh(&plant),
	generator(&plant),
	selection(&camera, &plant),
	rotateStem(&selection, &rotationAxes),
	moveStem(&selection, camera, 0, 0),
	movePath(&selection, &translationAxes),
	addStem(&selection)
{
	this->shared = shared;
	Vec3 color1 = {0.102f, 0.212f, 0.6f};
	Vec3 color2 = {0.102f, 0.212f, 0.6f};
	Vec3 color3 = {0.1f, 1.0f, 0.4f};
	path.setColor(color1, color2, color3);
	setMouseTracking(true);
	setFocus();

	extrudeCommand = false;
	addCommand = false;

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setAlignment(Qt::AlignBottom);
	QToolBar *toolbar = new QToolBar(this);
	connect(toolbar, SIGNAL(actionTriggered(QAction *)), this,
		SLOT(change(QAction *)));
	perspectiveAction = toolbar->addAction(tr("Perspective"));
	orthographicAction = toolbar->addAction(tr("Orthographic"));
	wireframeAction = toolbar->addAction(tr("Wireframe"));
	solidAction = toolbar->addAction(tr("Solid"));
	materialAction = toolbar->addAction(tr("Material"));
	perspectiveAction->setCheckable(true);
	orthographicAction->setCheckable(true);
	wireframeAction->setCheckable(true);
	solidAction->setCheckable(true);
	materialAction->setCheckable(true);
	perspectiveAction->toggle();
	solidAction->toggle();
	layout->addWidget(toolbar);

	perspective = true;
	shader = Model;
}

void Editor::initializeGL()
{
	initializeOpenGLFunctions();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(Geometry::primitiveReset);

	shared->initialize();
	generator.grow();
	initializeBuffers();
	change();
}

void Editor::initializeBuffers()
{
	Geometry geometry;
	Geometry axesLines = translationAxes.getLines();
	Geometry axesArrows = translationAxes.getArrows();
	Geometry rotationLines = rotationAxes.getLines();

	geometry.addGrid(5, {0.41, 0.41, 0.41}, {0.3, 0.3, 0.3});
	scene.grid = geometry.getSegment();
	scene.axesLines = geometry.append(axesLines);
	scene.axesArrows = geometry.append(axesArrows);
	scene.rotation = geometry.append(rotationLines);

	staticBuffer.initialize(GL_STATIC_DRAW);
	staticBuffer.load(geometry);

	plantBuffer.initialize(GL_DYNAMIC_DRAW);
	plantBuffer.allocatePointMemory(10);
	plantBuffer.allocateIndexMemory(10);

	pathBuffer.initialize(GL_DYNAMIC_DRAW);
	pathBuffer.allocatePointMemory(100);
	pathBuffer.allocateIndexMemory(100);
}

void Editor::keyPressEvent(QKeyEvent *event)
{
	bool ctrl = event->modifiers() & Qt::ControlModifier;
	switch (event->key()) {
	case Qt::Key_1:
		{
			SaveStemSelection selectionCopy(&selection);
			if (ctrl)
				selection.selectNextPoints();
			else
				selection.selectChildren();
			if (selectionCopy.hasChanged()) {
				selectionCopy.setAfter();
				history.add(selectionCopy);
				updateSelection();
				update();
				emit selectionChanged();
			}
		}
		break;
	case Qt::Key_2:
		{
			SaveStemSelection selectionCopy(&selection);
			if (ctrl)
				selection.selectPreviousPoints();
			else
				selection.selectSiblings();
			if (selectionCopy.hasChanged()) {
				selectionCopy.setAfter();
				history.add(selectionCopy);
				updateSelection();
				update();
				emit selectionChanged();
			}
		}
		break;
	case Qt::Key_3:
		{
			SaveStemSelection selectionCopy(&selection);
			if (ctrl)
				selection.selectAllPoints();
			else
				selection.selectAll();
			if (selectionCopy.hasChanged()) {
				selectionCopy.setAfter();
				history.add(selectionCopy);
				updateSelection();
				update();
				emit selectionChanged();
			}
		}
		break;
	case Qt::Key_4:
		if (!ctrl) {
			SaveStemSelection selectionCopy(&selection);
			selection.reduceToAncestors();
			if (selectionCopy.hasChanged()) {
				selectionCopy.setAfter();
				history.add(selectionCopy);
				emit selectionChanged();
				updateSelection();
				update();
			}
		}
		break;
	case Qt::Key_A:
		if (mode == None && selection.hasStems()) {
			QPoint p = mapFromGlobal(QCursor::pos());
			addStem.execute();
			clickOffset[0] = clickOffset[1] = 0;
			translationAxes.selectCenter();
			mode = PositionStem;
			addCommand = true;
			moveStem = MoveStem(&selection, camera, p.x(), p.y());
			moveStem.snapToCursor(true);
			moveStem.set(p.x(), p.y());
			moveStem.execute();
			change();
			update();
			emit selectionChanged();
		}
		break;
	case Qt::Key_C:
		if (mode == Rotate)
			rotationAxes.selectAxis(Axes::Center);
		break;
	case Qt::Key_E:
		if (mode == None && selection.hasPoints()) {
			mode = MovePoint;
			ExtrudeStem extrude(&selection);
			extrude.execute();
			history.add(extrude);
			extrudeCommand = true;
			QPoint p = mapFromGlobal(QCursor::pos());
			pg::Vec3 avg = selection.getAveragePosition();
			setClickOffset(p.x(), p.y(), avg);
			translationAxes.selectCenter();
			emit selectionChanged();
		}
		break;
	case Qt::Key_M:
		if (mode == None && selection.hasStems()) {
			QPoint p = mapFromGlobal(QCursor::pos());
			moveStem = MoveStem(&selection, camera, p.x(), p.y());
			mode = PositionStem;
		}
		break;
	case Qt::Key_R:
		if (selection.hasStems()) {
			QPoint p = mapFromGlobal(QCursor::pos());
			float x = p.x();
			float y = p.y();
			pg::Ray ray = camera.getRay(x, y);
			pg::Vec3 normal = camera.getDirection();
			rotateStem = RotateStem(&selection, &rotationAxes),
			rotateStem.set(ray, normal);
			mode = Rotate;
		}
		update();
		break;
	case Qt::Key_X:
		if (mode == Rotate)
			rotationAxes.selectAxis(Axes::XAxis);
		break;
	case Qt::Key_Y:
		if (mode == Rotate)
			rotationAxes.selectAxis(Axes::YAxis);
		break;
	case Qt::Key_Z:
		if (mode == Rotate)
			rotationAxes.selectAxis(Axes::ZAxis);
		break;
	case Qt::Key_Delete:
		if (selection.hasStems()) {
			mode = None;
			RemoveStem removeStem(&selection);
			removeStem.execute();
			history.add(removeStem);
			emit selectionChanged();
			updateSelection();
			change();
		}
		break;
	}

	QWidget::keyPressEvent(event);
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

	if (mode == Rotate) {
		mode = None;
		history.add(rotateStem);
		rotationAxes.selectCenter();
		update();
	} else if (event->button() == Qt::RightButton) {
		if (mode == None || mode == MovePoint) {
			SaveStemSelection selectionCopy(&selection);
			mode = None;
			selection.select(event);
			if (selectionCopy.hasChanged()) {
				selectionCopy.setAfter();
				history.add(selectionCopy);
				emit selectionChanged();
				updateSelection();
				update();
			}
		}
	} if (event->button() == Qt::MidButton && mode == None) {
		camera.setStartCoordinates(p.x(), p.y());
		if (event->modifiers() & Qt::ControlModifier)
			camera.setAction(Camera::Zoom);
		else if (event->modifiers() & Qt::ShiftModifier)
			camera.setAction(Camera::Pan);
		else
			camera.setAction(Camera::Rotate);
	} else if (event->button() == Qt::LeftButton) {
		if (selection.hasPoints() && mode == None) {
			movePath = MovePath(&selection, &translationAxes);
			selectAxis(p.x(), p.y());
		} else if (mode == PositionStem) {
			if (addCommand) {
				mode = MovePoint;
				movePath.set(camera, p.x(), p.y());
				movePath.execute();
				change();
			} else {
				mode = None;
				history.add(moveStem);
			}
		}
	}

	setFocus();
}

void Editor::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MidButton)
		camera.setAction(Camera::None);
	if (mode == MovePoint && event->button() == Qt::LeftButton) {
		translationAxes.clearSelection();
		if (addCommand) {
			history.add(addStem);
			addCommand = false;
		} else if (!extrudeCommand) {
			history.add(movePath);
		}
		extrudeCommand = false;
		mode = None;
	}
}

void Editor::mouseMoveEvent(QMouseEvent *event)
{
	QPoint point = event->pos();
	bool axisSelected = translationAxes.getSelection() != Axes::None;

	camera.executeAction(point.x(), point.y());

	if (mode == PositionStem) {
		moveStem.set(point.x(), point.y());
		moveStem.execute();
		change();
	} else if (mode == MovePoint && axisSelected) {
		bool ctrl = event->modifiers() & Qt::ControlModifier;
		float x = point.x() + clickOffset[0];
		float y = point.y() + clickOffset[1];
		movePath.setParallelTangents(!ctrl);
		movePath.set(camera, x, y);
		movePath.execute();
		change();
	} else if (mode == Rotate) {
		float x = point.x();
		float y = point.y();
		rotateStem.set(camera.getRay(x, y), camera.getDirection());
		rotateStem.execute();
		change();
	}

	update();
}

void Editor::selectAxis(int x, int y)
{
	if (!selection.hasPoint(0)) {
		pg::Ray ray = camera.getRay(x, y);
		float d;
		if (camera.isPerspective())
			d = pg::magnitude(camera.getPosition() -
				translationAxes.getPosition());
		else {
			d = pg::magnitude(camera.getFar() - camera.getNear());
			d /= 80.0f;
		}
		setClickOffset(x, y, translationAxes.getPosition());
		translationAxes.selectAxis(ray, d);
		if (translationAxes.getSelection() == Axes::Axis::None)
			mode = None;
		else
			mode = MovePoint;
	}
}

void Editor::setClickOffset(int x, int y, Vec3 point)
{
	Vec3 s = camera.toScreenSpace(point);
	clickOffset[0] = s.x - x;
	clickOffset[1] = s.y - y;
}

void Editor::resizeGL(int width, int height)
{
	updateCamera(width, height);
	translationAxes.setScale(600.0f / height);
}

void Editor::updateCamera(int width, int height)
{
	float ratio = static_cast<float>(width) / static_cast<float>(height);
	camera.setWindowSize(width, height);
	if (perspective)
		camera.setPerspective(45.0f, 0.1f, 200.0f, ratio);
	else {
		ratio /= 2.0f;
		Vec3 a = {-ratio, -0.5f, 0.0f};
		Vec3 b = {ratio, 0.5f, 100.0f};
		camera.setOrthographic(a, b);
	}
}

void Editor::paintGL()
{
	Mat4 vp = camera.getVP();
	Vec3 cp = camera.getPosition();
	GLsizei c = 0;
	GLvoid *s = 0;

	glDepthFunc(GL_LEQUAL);
	glClearColor(0.22f, 0.22f, 0.22f, 1.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* paint grid */
	staticBuffer.use();
	glUseProgram(shared->getShader(Shader::Flat));
	glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
	glDrawArrays(GL_LINES, scene.grid.pstart, scene.grid.pcount);

	/* paint plant */
	plantBuffer.use();
	if (shader == Shader::Model) {
		size_t start = 0;
		glUseProgram(shared->getShader(Shader::Model));
		glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
		glUniform4f(1, cp.x, cp.y, cp.z, 0.0f);
		for (int i = 0; i < mesh.getMeshCount(); i++) {
			c = mesh.getIndices(i)->size();
			s = (GLvoid *)start;
			start += mesh.getIndices(i)->size() * sizeof(unsigned);
			glDrawElements(GL_TRIANGLES, c, GL_UNSIGNED_INT, s);
		}
	} else if (shader == Shader::Wire) {
		size_t start = 0;
		glUseProgram(shared->getShader(Shader::Wire));
		glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
		glUniform4f(1, 0.13f, 0.13f, 0.13f, 1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		for (int i = 0; i < mesh.getMeshCount(); i++) {
			c = mesh.getIndices(i)->size();
			s = (GLvoid *)start;
			start += mesh.getIndices(i)->size() * sizeof(unsigned);
			glDrawElements(GL_TRIANGLES, c, GL_UNSIGNED_INT, s);
		}
	} else if (shader == Shader::Material) {
		size_t start = 0;
		glUseProgram(shared->getShader(Shader::Material));
		glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
		for (int i = 0; i < mesh.getMeshCount(); i++) {
			int materialId = mesh.getMaterialId(i);
			ShaderParams params = shared->getMaterial(materialId);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, params.getTexture(0));
			c = mesh.getIndices(i)->size();
			s = (GLvoid *)start;
			start += mesh.getIndices(i)->size() * sizeof(unsigned);
			glDrawElements(GL_TRIANGLES, c, GL_UNSIGNED_INT, s);
		}
	}

	if (shader != Shader::Wire) {
		glUseProgram(shared->getShader(Shader::Wire));
		glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(1, 0.3f, 0.3f, 0.3f, 1.0f);
	} else
		glUniform4f(1, 0.13f, 0.13f, 0.13f, 1.0f);

	for (unsigned i = 0; i < meshes.size(); i++) {
		s = (GLvoid *)(meshes[i].indexStart * sizeof(unsigned));
		c = meshes[i].indexCount;
		glDrawElements(GL_TRIANGLES, c, GL_UNSIGNED_INT, s);
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	/* Paint path lines */
	if (selection.hasStems()) {
		glDepthFunc(GL_ALWAYS);
		glPointSize(8);

		pathBuffer.use();
		glUseProgram(shared->getShader(Shader::Line));
		glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
		glUniform2f(1, QWidget::width(), QWidget::height());

		{
			Geometry::Segment segment = path.getLineSegment();
			s = (GLvoid *)(segment.istart * sizeof(unsigned));
			glDrawElements(GL_LINE_STRIP, segment.icount,
				GL_UNSIGNED_INT, s);
		}

		auto texture = shared->getTexture(shared->DotTexture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUseProgram(shared->getShader(Shader::Point));
		glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);

		{
			Geometry::Segment s = path.getPointSegment();
			glDrawArrays(GL_POINTS, s.pstart, s.pcount);
		}

		if (selection.hasPoints() || mode == Rotate)
			paintAxes();
	}

	glFlush();
}

void Editor::paintAxes()
{
	Mat4 vp = camera.getVP();
	Vec3 cp = camera.getPosition();
	GLvoid *s;
	GLsizei c;

	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LEQUAL);

	glUseProgram(shared->getShader(Shader::Flat));
	staticBuffer.use();

	translationAxes.setPosition(selection.getAveragePosition());

	float dist;
	if (camera.isPerspective())
		/* Both axes have the same position. */
		dist = pg::magnitude(cp - translationAxes.getPosition());
	else {
		dist = pg::magnitude(camera.getFar() - camera.getNear());
		dist /= 80.0f;
	}

	if (mode == Rotate) {
		pg::Vec3 dir = selection.getAverageDirectionFP();
		pg::Mat4 m = vp * rotationAxes.getTransformation(dist, dir);
		glUniformMatrix4fv(0, 1, GL_FALSE, &m[0][0]);
		s = (GLvoid *)((scene.rotation.istart) * sizeof(unsigned));
		c = scene.rotation.icount;
		glDrawElements(GL_LINE_STRIP, c, GL_UNSIGNED_INT, s);
	} else {
		pg::Mat4 m = vp * translationAxes.getTransformation(dist);
		glUniformMatrix4fv(0, 1, GL_FALSE, &m[0][0]);
		glDrawArrays(GL_LINES, scene.axesLines.pstart,
			scene.axesLines.pcount);
		s = (GLvoid *)((scene.axesArrows.istart) * sizeof(unsigned));
		c = scene.axesArrows.icount;
		glDrawElements(GL_TRIANGLES, c, GL_UNSIGNED_INT, s);
	}
}

/**
 * The path buffer, geometry segments, etc. are updated whenever the selection
 * changes.
 */
void Editor::updateSelection()
{
	meshes.clear();
	auto instances = selection.getInstances();
	for (auto &instance : instances)
		meshes.push_back(mesh.find(instance.first));

	if (!instances.empty()) {
		std::vector<Path::Segment> segments;
		for (auto &instance : instances) {
			pg::Stem *stem = instance.first;
			pg::Vec3 location = stem->getLocation();
			pg::Path path = stem->getPath();
			pg::Spline spline = path.getSpline();

			Path::Segment segment;
			segment.spline = spline;
			segment.resolution = path.getResolution();
			segment.location = location;
			segments.push_back(segment);
		}
		path.set(segments);

		{
			int i = 0;
			auto instances = selection.getInstances();
			for (auto &instance : instances)
				path.setSelectedPoints(instance.second, i++);
		}

		const Geometry *geometry = path.getGeometry();
		pathBuffer.update(*geometry);
	}
}

void Editor::change()
{
	if (!isValid())
	 	return;

	mesh.generate();
	plantBuffer.use();
	if (mesh.getVertexCount() > plantBuffer.getCapacity(Buffer::Points))
		plantBuffer.allocatePointMemory(mesh.getVertexCount() * 2);
	if (mesh.getIndexCount() > plantBuffer.getCapacity(Buffer::Indices))
		plantBuffer.allocateIndexMemory(mesh.getIndexCount() * 2);

	int pointOffset = 0;
	int indexOffset = 0;
	for (int m = 0; m < mesh.getMeshCount(); m++) {
		const std::vector<float> *v = mesh.getVertices(m);
		const std::vector<unsigned> *i = mesh.getIndices(m);
		if (!plantBuffer.update(v->data(), pointOffset, v->size())) {
			assert("Failed to update vertices.");
			exit(0);
		}
		if (!plantBuffer.update(i->data(), indexOffset, i->size())) {
			assert("Failed to update indices.");
			exit(0);
		}
		pointOffset += v->size();
		indexOffset += i->size();
	}

	updateSelection();
	update();
}

void Editor::load(const char *filename)
{
	plant.removeRoot();
	if (filename == nullptr)
		generator.grow();
	else {
		std::ifstream stream(filename);
		boost::archive::text_iarchive ia(stream);
		ia >> plant;
		stream.close();
	}
	selection.clear();
	history.clear();
	emit selectionChanged();
	change();
}

void Editor::change(QAction *action)
{
	QString text = action->text();
	if (text == "Perspective") {
		perspective = true;
		orthographicAction->setChecked(false);
		perspectiveAction->setChecked(true);
		updateCamera(width(), height());
	} else if (text == "Orthographic") {
		perspective = false;
		orthographicAction->setChecked(true);
		perspectiveAction->setChecked(false);
		updateCamera(width(), height());
	} else if (text == "Wireframe") {
		shader = Wire;
		wireframeAction->setChecked(true);
		solidAction->setChecked(false);
		materialAction->setChecked(false);
	} else if (text == "Solid") {
		shader= Model;
		wireframeAction->setChecked(false);
		solidAction->setChecked(true);
		materialAction->setChecked(false);
	} else if (text == "Material") {
		shader = Material;
		wireframeAction->setChecked(false);
		solidAction->setChecked(false);
		materialAction->setChecked(true);
	}
	update();
}

void Editor::updateMaterial(ShaderParams params)
{
	plant.addMaterial(params.getMaterial());
}

pg::Plant *Editor::getPlant()
{
	return &plant;
}

StemSelection *Editor::getSelection()
{
	return &selection;
}

const pg::Mesh *Editor::getMesh()
{
	return &mesh;
}

void Editor::add(Command &cmd)
{
	history.add(cmd);
}

void Editor::undo()
{
	if (mode == None) {
		history.undo();
		change();
		emit selectionChanged();
	}
}

void Editor::redo()
{
	if (mode == None) {
		history.redo();
		change();
		emit selectionChanged();
	}
}
