/* Plant Generator
 * Copyright (C) 2016-2018  Floris Creyf
 *
 * Plant Generator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Generator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "editor.h"
#include "editor/selector.h"
#include "editor/commands/add_leaf.h"
#include "editor/commands/add_stem.h"
#include "editor/commands/extrude_stem.h"
#include "editor/commands/move_stem.h"
#include "editor/commands/move_path.h"
#include "editor/commands/remove_stem.h"
#include "editor/commands/rotate_stem.h"
#include "editor/geometry/geometry.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>

#include <QMenu>
#include <QScrollArea>
#include <QtOpenGL/QGLFormat>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QVBoxLayout>

#include <boost/archive/text_iarchive.hpp>

using pg::Vec3;
using pg::Mat4;
using pg::Ray;

Editor::Editor(SharedResources *shared, KeyMap *keymap, QWidget *parent) :
	QOpenGLWidget(parent),
	generator(&scene.plant),
	mesh(&scene.plant),
	selection(&scene.plant)
{
	this->shared = shared;
	this->keymap = keymap;

	currentCommand = nullptr;
	perspective = true;
	shader = SharedResources::Model;

	Vec3 color1(0.102f, 0.212f, 0.6f);
	Vec3 color2(0.102f, 0.212f, 0.6f);
	Vec3 color3(0.1f, 1.0f, 0.4f);
	path.setColor(color1, color2, color3);
	setMouseTracking(true);
	setFocus();

	createToolBar();
}

void Editor::createToolBar()
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setAlignment(Qt::AlignBottom | Qt::AlignRight);
	QToolBar *toolbar = new QToolBar(this);
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

	connect(toolbar, SIGNAL(actionTriggered(QAction *)),
		this, SLOT(change(QAction *)));
}

void Editor::initializeGL()
{
	initializeOpenGLFunctions();

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glEnable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(Geometry::primitiveReset);

	createFrameBuffers();
	shared->initialize();
	initializeBuffers();

	change();
}

void Editor::createFrameBuffers()
{
	glGenFramebuffers(1, &outlineFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, outlineFrameBuffer);
	glGenTextures(1, &outlineColorMap);
	glBindTexture(GL_TEXTURE_2D, outlineColorMap);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGB, width(), height(), 0, GL_RGB,
		GL_UNSIGNED_BYTE, NULL);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COPY);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, outlineColorMap, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Editor::initializeBuffers()
{
	Geometry geometry;
	Geometry axesLines = translationAxes.getLines();
	Geometry axesArrows = translationAxes.getArrows();
	Geometry rotationLines = rotationAxes.getLines();

	Vec3 colors[2];
	colors[0] = Vec3(0.4f, 0.4f, 0.4f);
	colors[1] = Vec3(0.4f, 0.4f, 0.4f);
	geometry.addGrid(5, colors, Vec3(0.3, 0.3, 0.3));
	segments.grid = geometry.getSegment();
	segments.axesLines = geometry.append(axesLines);
	segments.axesArrows = geometry.append(axesArrows);
	segments.rotation = geometry.append(rotationLines);

	staticBuffer.initialize(GL_STATIC_DRAW);
	staticBuffer.load(geometry);

	plantBuffer.initialize(GL_DYNAMIC_DRAW);
	plantBuffer.allocatePointMemory(1000);
	plantBuffer.allocateIndexMemory(1000);

	pathBuffer.initialize(GL_DYNAMIC_DRAW);
	pathBuffer.allocatePointMemory(100);
	pathBuffer.allocateIndexMemory(100);
}

void Editor::keyPressEvent(QKeyEvent *event)
{
	if (currentCommand) {
		exitCommand(currentCommand->onKeyPress(event));
		QWidget::keyPressEvent(event);
		return;
	}

	QPoint pos = mapFromGlobal(QCursor::pos());
	float x = pos.x();
	float y = pos.y();

	unsigned key = event->key();
	bool ctrl = event->modifiers() & Qt::ControlModifier;
	bool shift = event->modifiers() & Qt::ShiftModifier;
	bool alt = event->modifiers() & Qt::AltModifier;
	QString command = keymap->getBinding(key, ctrl, shift, alt);

	pg::Stem *root = scene.plant.getRoot();
	bool hasPoints = selection.hasPoints();
	bool hasFirstPoint = selection.hasPoint(0);
	bool hasStems = selection.hasStems();
	bool hasOneStem = selection.getStemInstances().size() == 1;
	bool hasLeaves = selection.hasLeaves();
	bool containsRoot = selection.contains(root);

	if (command == tr("Add Leaf") && hasOneStem) {
		currentCommand = new AddLeaf(&selection, &camera, x, y);
		currentCommand->execute();
		change();
		emit selectionChanged();
	} else if (command == tr("Add Stem") && (hasOneStem || !root)) {
		currentCommand = new AddStem(
			&selection, &translationAxes, &camera, x, y);
		currentCommand->execute();
		change();
		emit selectionChanged();
	} else if (command == tr("Extrude") && hasPoints) {
		ExtrudeStem *extrude = new ExtrudeStem(
			&selection, &translationAxes, &camera);
		extrude->setClickOffset(x, y);
		extrude->execute();
		currentCommand = extrude;
		emit selectionChanged();
	} else if (command == tr("Move Point") && hasPoints && !hasFirstPoint) {
		MovePath *movePath = new MovePath(
			&selection, &translationAxes, &camera);
		Vec3 axesPosition = translationAxes.getPosition();
		Vec3 crd = camera.toScreenSpace(axesPosition);
		movePath->setClickOffset(crd.x - x, crd.y - y);
		currentCommand = movePath;
	} else if (command == tr("Move Stem") &&
			(hasStems || hasLeaves) && !containsRoot) {
		currentCommand = new MoveStem(&selection, &camera, x, y);
	} else if (command == tr("Reduce To Ancestors") && hasStems) {
		SaveSelection *copy = new SaveSelection(&selection);
		selection.reduceToAncestors();
		addSelectionToHistory(copy);
	} else if (command == tr("Remove") && (hasStems || hasLeaves)) {
		RemoveStem *removeStem = new RemoveStem(&selection);
		removeStem->execute();
		history.add(removeStem);
		emit selectionChanged();
		updateSelection();
		change();
	} else if (command == tr("Rotate") && (hasStems || hasLeaves)) {
		RotateStem *rotate = new RotateStem(
			&selection, &rotationAxes, &camera, x, y);
		currentCommand = rotate;
		rotating = true;
	} else if (command == tr("Select Points")) {
		SaveSelection *copy = new SaveSelection(&selection);
		selection.selectAllPoints();
		addSelectionToHistory(copy);
	} else if (command == tr("Select Children")) {
		SaveSelection *copy = new SaveSelection(&selection);
		selection.selectChildren();
		addSelectionToHistory(copy);
	} else if (command == tr("Select Leaves")) {
		SaveSelection *copy = new SaveSelection(&selection);
		selection.selectLeaves();
		selection.removeStems();
		addSelectionToHistory(copy);
	} else if (command == tr("Select Previous Points")) {
		SaveSelection *copy = new SaveSelection(&selection);
		selection.selectPreviousPoints();
		addSelectionToHistory(copy);
	} else if (command == tr("Select Next Points")) {
		SaveSelection *copy = new SaveSelection(&selection);
		selection.selectNextPoints();
		addSelectionToHistory(copy);
	} else if (command == tr("Select Siblings")) {
		SaveSelection *copy = new SaveSelection(&selection);
		selection.selectSiblings();
		addSelectionToHistory(copy);
	} else if (command == tr("Select Stems")) {
		SaveSelection *copy = new SaveSelection(&selection);
		selection.selectStems();
		addSelectionToHistory(copy);
	}

	update();
	QWidget::keyPressEvent(event);
}

void Editor::exitCommand(bool changed)
{
	if (changed)
		change();

	if (currentCommand->isDone()) {
		history.add(currentCommand);
		currentCommand = nullptr;
		rotating = false;
	}
}

void Editor::addSelectionToHistory(SaveSelection *selection)
{
	if (selection->hasChanged()) {
		selection->setAfter();
		history.add(selection);
		emit selectionChanged();
		updateSelection();
		update();
	}
}

void Editor::wheelEvent(QWheelEvent *event)
{
	QPoint a = event->angleDelta();
	if (!a.isNull()) {
		float y = a.y() / 8.0f;
		if (y != 0.0f) {
			camera.zoom(y);
			update();
		}
	}
	event->accept();
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
	QPoint pos = event->pos();
	if (currentCommand) {
		exitCommand(currentCommand->onMousePress(event));
	} else if (event->button() == Qt::RightButton) {
		SaveSelection *selectionCopy = new SaveSelection(&selection);
		Selector selector(&this->camera);
		selector.select(event, &this->mesh, &this->selection);
		if (selectionCopy->hasChanged()) {
			selectionCopy->setAfter();
			history.add(selectionCopy);
			emit selectionChanged();
			updateSelection();
			update();
		}
	} else if (event->button() == Qt::MidButton) {
		camera.setStartCoordinates(pos.x(), pos.y());
		if (event->modifiers() & Qt::ControlModifier)
			camera.setAction(Camera::Zoom);
		else if (event->modifiers() & Qt::ShiftModifier)
			camera.setAction(Camera::Pan);
		else
			camera.setAction(Camera::Rotate);
	} else if (event->button() == Qt::LeftButton) {
		selectAxis(pos.x(), pos.y());
		bool axis = translationAxes.getSelection();
		bool hasPoints = selection.hasPoints();
		bool hasFirstPoint = selection.hasPoint(0);
		if (hasPoints && !hasFirstPoint && axis != Axes::None) {
			MovePath *movePath = new MovePath(
				&selection, &translationAxes, &camera);
			Vec3 axesPosition = translationAxes.getPosition();
			Vec3 s = camera.toScreenSpace(axesPosition);
			movePath->setClickOffset(s.x - pos.x(), s.y - pos.y());
			currentCommand = movePath;
		}
	}
	setFocus();
}

void Editor::mouseReleaseEvent(QMouseEvent *event)
{
	if (currentCommand)
		exitCommand(currentCommand->onMouseRelease(event));
	else if (event->button() == Qt::MidButton)
		camera.setAction(Camera::None);
}

void Editor::mouseMoveEvent(QMouseEvent *event)
{
	if (currentCommand)
		exitCommand(currentCommand->onMouseMove(event));
	else {
		QPoint point = event->pos();
		camera.executeAction(point.x(), point.y());
		update();
	}
}

void Editor::selectAxis(int x, int y)
{
	if (selection.hasPoint(0))
		return;

	pg::Ray ray = camera.getRay(x, y);
	float distance;
	if (camera.isPerspective()) {
		Vec3 cameraPosition = camera.getPosition();
		Vec3 axesPosition = translationAxes.getPosition();
		distance = pg::magnitude(cameraPosition - axesPosition);
	} else {
		distance = pg::magnitude(camera.getFar() - camera.getNear());
		distance /= 80.0f;
	}
	translationAxes.selectAxis(ray, distance);
}

void Editor::resizeGL(int width, int height)
{
	updateCamera(width, height);
	translationAxes.setScale(600.0f / height);
	glBindTexture(GL_TEXTURE_2D, outlineColorMap);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
		GL_UNSIGNED_BYTE, NULL);
}

void Editor::updateCamera(int width, int height)
{
	float ratio = static_cast<float>(width) / static_cast<float>(height);
	camera.setWindowSize(width, height);
	if (perspective)
		camera.setPerspective(45.0f, 0.001f, 200.0f, ratio);
	else {
		ratio /= 2.0f;
		Vec3 a(-ratio, -0.5f, 0.0f);
		Vec3 b(ratio, 0.5f, 100.0f);
		camera.setOrthographic(a, b);
	}
}

void Editor::paintGL()
{
	Mat4 projection = camera.updateVP();
	Vec3 position = camera.getPosition();

	/* Render the scene normally. */
	glClearDepth(1.0f);
	glClearColor(0.22f, 0.23f, 0.24f, 1.0);
	glDepthFunc(GL_LEQUAL);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

	/* Paint the grid. */
	staticBuffer.use();
	glUseProgram(shared->getShader(SharedResources::Flat));
	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	glDrawArrays(GL_LINES, segments.grid.pstart, segments.grid.pcount);

	/* Paint the plant. */
	plantBuffer.use();
	if (shader == SharedResources::Model)
		paintModel(projection, position);
	else if (shader == SharedResources::Wire)
		paintWire(projection);
	else if (shader == SharedResources::Material)
		paintMaterial(projection);

	/* Create a texture for selection objects. */
	glBindFramebuffer(GL_FRAMEBUFFER, outlineFrameBuffer);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width(), height());

	glUseProgram(shared->getShader(SharedResources::Outline));
	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	glUniform4f(1, position.x, position.y, position.z, 0.0f);
	glUniform1i(2, 0);
	glUniform2f(3, (GLfloat)width(), (GLfloat)height());
	for (unsigned i = 0; i < meshes.size(); i++) {
		unsigned index = meshes[i].indexStart;
		GLvoid *offset = (GLvoid *)(index * sizeof(unsigned));
		GLsizei size = meshes[i].indexCount;
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, offset);
	}

	/* Draw objects to screen with outline. */
	GLuint defaultFramebuffer = this->context()->defaultFramebufferObject();
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
	glBindTexture(GL_TEXTURE_2D, outlineColorMap);
	glUniform1i(2, 1);
	for (unsigned i = 0; i < meshes.size(); i++) {
		unsigned index = meshes[i].indexStart;
		GLvoid *offset = (GLvoid *)(index * sizeof(unsigned));
		GLsizei size = meshes[i].indexCount;
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, offset);
	}

	/* Paint path lines. */
	if (selection.hasStems()) {
		Geometry::Segment segment;

		glDepthFunc(GL_ALWAYS);
		glPointSize(6);

		pathBuffer.use();
		glUseProgram(shared->getShader(SharedResources::Line));
		glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
		glUniform2f(1, QWidget::width(), QWidget::height());

		segment = path.getLineSegment();
		GLvoid *offset = (GLvoid *)(segment.istart * sizeof(unsigned));
		glDrawElements(
			GL_LINE_STRIP, segment.icount,
			GL_UNSIGNED_INT, offset);

		auto texture = shared->getTexture(shared->DotTexture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUseProgram(shared->getShader(SharedResources::Point));
		glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);

		segment = path.getPointSegment();
		glDrawArrays(GL_POINTS, segment.pstart, segment.pcount);

		if (selection.hasPoints())
			paintAxes();
	} else if (selection.hasLeaves())
		paintAxes();

	glFlush();
}

void Editor::paintWire(const Mat4 &projection)
{
	glUseProgram(shared->getShader(SharedResources::Wire));
	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	glUniform4f(1, 0.13f, 0.13f, 0.13f, 1.0f);

	glPointSize(4);
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	for (size_t i = 0; i < mesh.getMeshCount(); i++) {
		GLsizei size = mesh.getVertices(i)->size();
		glDrawArrays(GL_POINTS, 0, size);
	}

	size_t start = 0;
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	for (size_t i = 0; i < mesh.getMeshCount(); i++) {
		GLsizei size = mesh.getIndices(i)->size();
		glDrawElements(GL_TRIANGLES, size,
			GL_UNSIGNED_INT, (GLvoid *)start);
		start += mesh.getIndices(i)->size() * sizeof(unsigned);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Editor::paintModel(const Mat4 &projection, const Vec3 &position)
{
	size_t start = 0;
	glUseProgram(shared->getShader(SharedResources::Model));
	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	glUniform4f(1, position.x, position.y, position.z, 0.0f);
	for (size_t i = 0; i < mesh.getMeshCount(); i++) {
		GLsizei size = mesh.getIndices(i)->size();
		glDrawElements(GL_TRIANGLES, size,
			GL_UNSIGNED_INT, (GLvoid *)start);
		start += mesh.getIndices(i)->size() * sizeof(unsigned);
	}
}

void Editor::paintMaterial(const Mat4 &projection)
{
	size_t start = 0;
	glUseProgram(shared->getShader(SharedResources::Material));
	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	for (size_t i = 0; i < mesh.getMeshCount(); i++) {
		unsigned index = mesh.getMaterialIndex(i);
		ShaderParams params = shared->getMaterial(index);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, params.getTexture(0));
		GLsizei size = mesh.getIndices(i)->size();
		glDrawElements(GL_TRIANGLES, size,
			GL_UNSIGNED_INT, (GLvoid *)start);
		start += mesh.getIndices(i)->size() * sizeof(unsigned);
	}
}

void Editor::paintAxes()
{
	Mat4 projection = camera.getVP();
	Vec3 position = camera.getPosition();

	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LEQUAL);

	glUseProgram(shared->getShader(SharedResources::Flat));
	staticBuffer.use();

	translationAxes.setPosition(selection.getAveragePosition());

	float distance;
	if (camera.isPerspective()) {
		Vec3 diff = position - translationAxes.getPosition();
		distance = pg::magnitude(diff);
	} else {
		distance = pg::magnitude(camera.getFar() - camera.getNear());
		distance /= 80.0f;
	}

	if (rotating) {
		Vec3 direction = selection.getAverageDirectionFP();
		Mat4 transformation =
			projection *
			rotationAxes.getTransformation(distance, direction);
		glUniformMatrix4fv(0, 1, GL_FALSE, &transformation[0][0]);
		unsigned index = segments.rotation.istart;
		GLvoid *offset = (GLvoid *)(index * sizeof(unsigned));
		GLsizei size = segments.rotation.icount;
		glDrawElements(GL_LINE_STRIP, size, GL_UNSIGNED_INT, offset);
	} else {
		Mat4 transformation = projection;
		transformation *= translationAxes.getTransformation(distance);
		glUniformMatrix4fv(0, 1, GL_FALSE, &transformation[0][0]);
		glDrawArrays(
			GL_LINES, segments.axesLines.pstart,
			segments.axesLines.pcount);
		unsigned index = segments.axesArrows.istart;
		GLvoid *offset = (GLvoid *)(index * sizeof(unsigned));
		GLsizei size = segments.axesArrows.icount;
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, offset);
	}
}

/** Determine what regions in the buffer are the selection. */
void Editor::updateSelection()
{
	meshes.clear();
	auto stemInstances = selection.getStemInstances();
	for (auto &instance : stemInstances)
		meshes.push_back(mesh.findStem(instance.first));
	auto leafInstances = selection.getLeafInstances();
	for (auto &instance : leafInstances) {
		for (auto &leaf : instance.second) {
			pg::Mesh::LeafID id(instance.first, leaf);
			meshes.push_back(mesh.findLeaf(id));
		}
	}

	if (!stemInstances.empty()) {
		std::vector<Path::Segment> segments;
		for (auto &instance : stemInstances) {
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

		int i = 0;
		auto instances = selection.getStemInstances();
		for (auto &instance : instances)
			path.setSelectedPoints(instance.second, i++);

		const Geometry *geometry = path.getGeometry();
		makeCurrent();
		pathBuffer.update(*geometry);
		doneCurrent();
	}
}

/** Update the graphics buffer. */
void Editor::change()
{
	if (!isValid())
		return;

	scene.wind.generate(&scene.plant);
	mesh.generate();

	makeCurrent();
	plantBuffer.use();
	if (mesh.getVertexCount() > plantBuffer.getCapacity(Buffer::Points))
		plantBuffer.allocatePointMemory(mesh.getVertexCount() * 2);
	if (mesh.getIndexCount() > plantBuffer.getCapacity(Buffer::Indices))
		plantBuffer.allocateIndexMemory(mesh.getIndexCount() * 2);

	int pointOffset = 0;
	int indexOffset = 0;
	for (size_t m = 0; m < mesh.getMeshCount(); m++) {
		const std::vector<pg::Vertex> *v = mesh.getVertices(m);
		const std::vector<unsigned> *i = mesh.getIndices(m);
		if (!plantBuffer.update(v->data(), pointOffset, v->size()))
			throw "failed to update vertices\n";
		if (!plantBuffer.update(i->data(), indexOffset, i->size()))
			throw "failed to update indices\n";
		pointOffset += v->size();
		indexOffset += i->size();
	}
	doneCurrent();

	updateSelection();
	update();
	emit changed();
}

void Editor::createDefaultPlant()
{
	pg::Derivation derivation = generator.getDerivation();
	std::random_device rd;
	derivation.seed = rd();
	derivation.depth = 1;
	derivation.stemDensity = 1.0f;
	derivation.stemDensityCurve.setDefault(1);
	derivation.stemStart = 2.0f;
	derivation.leafDensity = 0.5f;
	derivation.leafDensityCurve.setDefault(1);
	derivation.leafStart = 1.0f;
	derivation.lengthFactor = 50.0f;
	generator.setDerivation(derivation);
	generator.grow();
}

void Editor::load(const char *filename)
{
	scene.plant.removeRoot();
	if (filename == nullptr)
		createDefaultPlant();
	else {
		std::ifstream stream(filename);
		boost::archive::text_iarchive ia(stream);
		ia >> scene;
		stream.close();
	}
}

void Editor::reset()
{
	selection.clear();
	history.clear();
	if (isValid())
		change();
	emit selectionChanged();
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
		shader = SharedResources::Wire;
		wireframeAction->setChecked(true);
		solidAction->setChecked(false);
		materialAction->setChecked(false);
	} else if (text == "Solid") {
		shader = SharedResources::Model;
		wireframeAction->setChecked(false);
		solidAction->setChecked(true);
		materialAction->setChecked(false);
	} else if (text == "Material") {
		shader = SharedResources::Material;
		wireframeAction->setChecked(false);
		solidAction->setChecked(false);
		materialAction->setChecked(true);
	}
	update();
}

void Editor::updateMaterial(unsigned index)
{
	ShaderParams params = this->shared->getMaterial(index);
	scene.plant.updateMaterial(params.getMaterial(), index);
}

pg::Plant *Editor::getPlant()
{
	return &scene.plant;
}

pg::Scene *Editor::getScene()
{
	return &scene;
}

Selection *Editor::getSelection()
{
	return &selection;
}

const pg::Mesh *Editor::getMesh()
{
	return &mesh;
}

void Editor::add(Command *cmd)
{
	history.add(cmd);
}

void Editor::undo()
{
	if (!currentCommand) {
		history.undo();
		change();
		emit selectionChanged();
	}
}

void Editor::redo()
{
	if (!currentCommand) {
		history.redo();
		change();
		emit selectionChanged();
	}
}
