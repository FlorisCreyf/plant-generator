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
#include <boost/archive/text_iarchive.hpp>

#undef near
#undef far

using pg::Mat4;
using pg::Vec3;
using pg::Stem;
using pg::Material;
using pg::Mesh;
using pg::DVertex;
using std::vector;

const float pi = 3.14159265359f;

Editor::Editor(SharedResources *shared, KeyMap *keymap, QWidget *parent) :
	QOpenGLWidget(parent),
	timer(new QTimer(this)),
	command(nullptr),
	keymap(keymap),
	shared(shared),
	shader(SharedResources::Solid),
	meshGenerator(&scene.plant),
	mesh(meshGenerator.getMesh()),
	selection(&scene.plant),
	updatedLight(false),
	perspective(true),
	rotating(false),
	ticks(-1)
{
	Vec3 color1(0.102f, 0.212f, 0.6f);
	Vec3 color2(0.102f, 0.212f, 0.6f);
	Vec3 color3(0.1f, 1.0f, 0.4f);
	this->path.setColor(color1, color2, color3);
	this->camera.setOrientation(pi*0.45f, 0.0f);
	this->camera.setDistance(15.0f);
	createToolBar();
	setMouseTracking(true);
	setFocus();
	this->timer->setInterval(33);
	connect(this->timer, &QTimer::timeout, this, &Editor::animate);
}

void Editor::createToolBar()
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setAlignment(Qt::AlignBottom | Qt::AlignRight);
	QToolBar *toolbar = new QToolBar(this);
	this->perspectiveAction = toolbar->addAction("Perspective");
	this->orthographicAction = toolbar->addAction("Orthographic");
	this->wireframeAction = toolbar->addAction("Wireframe");
	this->solidAction = toolbar->addAction("Solid");
	this->materialAction = toolbar->addAction("Material");
	this->perspectiveAction->setCheckable(true);
	this->orthographicAction->setCheckable(true);
	this->wireframeAction->setCheckable(true);
	this->solidAction->setCheckable(true);
	this->materialAction->setCheckable(true);
	this->perspectiveAction->toggle();
	this->solidAction->toggle();
	layout->addWidget(toolbar);
	connect(toolbar, QOverload<QAction *>::of(&QToolBar::actionTriggered),
		this, QOverload<QAction *>::of(&Editor::change));
}

void Editor::initializeGL()
{
	initializeOpenGLFunctions();
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(Geometry::primitiveReset);
	createFramebuffers(true);
	this->shared->initialize();
	initializeBuffers();
	change();
}

void Editor::createFramebuffers(bool initial)
{
	glGenFramebuffers(1, &this->msSilhouetteFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, this->msSilhouetteFramebuffer);
	glGenTextures(1, &this->msSilhouetteMap);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, this->msSilhouetteMap);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB,
		width(), height(), GL_TRUE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D_MULTISAMPLE, this->msSilhouetteMap, 0);

	glGenFramebuffers(1, &this->silhouetteFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, this->silhouetteFramebuffer);
	glGenTextures(1, &this->silhouetteMap);
	glBindTexture(GL_TEXTURE_2D, this->silhouetteMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width(), height(), 0, GL_RGB,
		GL_UNSIGNED_BYTE, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, this->silhouetteMap, 0);

	if (!initial)
		return;

	this->shadowMapSize = 2048;
	glGenFramebuffers(1, &this->shadowFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, this->shadowFramebuffer);
	glGenTextures(1, &this->shadowMap);
	glBindTexture(GL_TEXTURE_2D, this->shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->shadowMapSize,
		this->shadowMapSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat border[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, this->shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
}

void Editor::deleteFramebuffers()
{
	glDeleteFramebuffers(1, &this->msSilhouetteFramebuffer);
	glDeleteTextures(1, &this->msSilhouetteMap);
	glDeleteFramebuffers(1, &this->silhouetteFramebuffer);
	glDeleteTextures(1, &this->silhouetteMap);
}

void Editor::initializeBuffers()
{
	Geometry geometry;
	Geometry axesLines = this->translationAxes.getLines();
	Geometry axesArrows = this->translationAxes.getArrows();
	Geometry rotationLines = this->rotationAxes.getLines();

	Geometry plane;
	plane.addPlane(
		Vec3(2.0f, 0.0f, 0.0f),
		Vec3(0.0f, 2.0f, 0.0f),
		Vec3(-1.0f, -1.0f, 1.0f),
		Vec3(0.0f, 0.0f, 0.0f),
		Vec3(0.0f, 0.0f, 0.0f));

	Vec3 colors[2];
	colors[0] = Vec3(0.7f, 0.4f, 0.4f);
	colors[1] = Vec3(0.4f, 0.7f, 0.4f);
	geometry.addGrid(5, colors, Vec3(0.3f, 0.3f, 0.3f));
	this->segments.grid = geometry.getSegment();
	this->segments.plane = geometry.append(plane);
	this->segments.axesLines = geometry.append(axesLines);
	this->segments.axesArrows = geometry.append(axesArrows);
	this->segments.rotation = geometry.append(rotationLines);
	this->segments.volume = {};

	this->staticBuffer.initialize(GL_STATIC_DRAW);
	this->staticBuffer.load(geometry);
	this->plantBuffer.initialize(GL_DYNAMIC_DRAW);
	this->plantBuffer.allocatePointMemory(1000);
	this->plantBuffer.allocateIndexMemory(1000);
	this->pathBuffer.initialize(GL_DYNAMIC_DRAW);
	this->pathBuffer.allocatePointMemory(100);
	this->pathBuffer.allocateIndexMemory(100);
	this->volumeBuffer.initialize(GL_DYNAMIC_DRAW);
	this->volumeBuffer.allocatePointMemory(100);
	this->jointBuffer.initialize(GL_DYNAMIC_DRAW, 5);
}

void Editor::keyPressEvent(QKeyEvent *event)
{
	if (this->command) {
		QWidget::keyPressEvent(event);
		exitCommand(this->command->onKeyPress(event));
		return;
	} else if (this->scene.updating)
		return;

	Stem *root = this->scene.plant.getRoot();
	QPoint pos = mapFromGlobal(QCursor::pos());
	float x = pos.x();
	float y = pos.y();
	QString commandName = this->keymap->getBinding(
		event->key(),
		event->modifiers() & Qt::ControlModifier,
		event->modifiers() & Qt::ShiftModifier,
		event->modifiers() & Qt::AltModifier);

	if (commandName == "Add Leaf") {
		size_t stemCount = this->selection.getStemInstances().size();
		if (stemCount == 1) {
			this->command = new AddLeaf(
				&this->selection,
				&this->camera, x, y);
			this->command->execute();
			change();
			emit selectionChanged();
		}
	} else if (commandName == "Add Stem") {
		size_t stemCount = this->selection.getStemInstances().size();
		if (stemCount == 1 || !root) {
			this->command = new AddStem(
				&this->selection,
				&this->translationAxes,
				&this->camera, x, y);
			this->command->execute();
			change();
			emit selectionChanged();
		}
	} else if (commandName == "Extrude") {
		if (this->selection.hasPoints()) {
			ExtrudeStem *extrude = new ExtrudeStem(
				&this->selection,
				&this->translationAxes,
				&this->camera);
			extrude->setClickOffset(x, y);
			extrude->execute();
			this->command = extrude;
			emit selectionChanged();
		}
	} else if (commandName == "Move Point") {
		bool hasFirstPoint = this->selection.hasPoint(0);
		if (this->selection.hasPoints() && !hasFirstPoint) {
			MovePath *movePath = new MovePath(
				&this->selection,
				&this->translationAxes,
				&this->camera);
			Vec3 axesPosition = this->translationAxes.getPosition();
			Vec3 crd = this->camera.toScreenSpace(axesPosition);
			movePath->setClickOffset(crd.x - x, crd.y - y);
			this->command = movePath;
		}
	} else if (commandName == "Move Stem") {
		bool hasStems = this->selection.hasStems();
		bool hasLeaves = this->selection.hasLeaves();
		bool containsRoot = this->selection.contains(root);
		if ((hasStems || hasLeaves) && !containsRoot)
			this->command = new MoveStem(
				&this->selection,
				&this->camera, x, y);
	} else if (commandName == "Reduce To Ancestors") {
		if (this->selection.hasStems()) {
			SaveSelection *copy;
			copy = new SaveSelection(&this->selection);
			this->selection.reduceToAncestors();
			addSelectionToHistory(copy);
		}
	} else if (commandName == "Remove") {
		bool hasStems = this->selection.hasStems();
		bool hasLeaves = this->selection.hasLeaves();
		if (hasStems || hasLeaves) {
			RemoveStem *removeStem;
			removeStem = new RemoveStem(&this->selection);
			removeStem->execute();
			this->history.add(removeStem);
			emit selectionChanged();
			updateSelection();
			change();
		}
	} else if (commandName == "Rotate") {
		bool hasStems = this->selection.hasStems();
		bool hasLeaves = this->selection.hasLeaves();
		if (hasStems || hasLeaves) {
			RotateStem *rotate = new RotateStem(
				&this->selection,
				&this->rotationAxes,
				&this->camera, x, y);
			this->command = rotate;
			this->rotating = true;
		}
	} else if (commandName == "Select Points") {
		SaveSelection *copy = new SaveSelection(&this->selection);
		this->selection.selectAllPoints();
		addSelectionToHistory(copy);
	} else if (commandName == "Select Children") {
		SaveSelection *copy = new SaveSelection(&this->selection);
		selection.selectChildren();
		addSelectionToHistory(copy);
	} else if (commandName == "Select Leaves") {
		SaveSelection *copy = new SaveSelection(&this->selection);
		selection.selectLeaves();
		selection.removeStems();
		addSelectionToHistory(copy);
	} else if (commandName == "Select Previous Points") {
		SaveSelection *copy = new SaveSelection(&this->selection);
		this->selection.selectPreviousPoints();
		addSelectionToHistory(copy);
	} else if (commandName == "Select Next Points") {
		SaveSelection *copy = new SaveSelection(&this->selection);
		this->selection.selectNextPoints();
		addSelectionToHistory(copy);
	} else if (commandName == "Select Siblings") {
		SaveSelection *copy = new SaveSelection(&this->selection);
		this->selection.selectSiblings();
		addSelectionToHistory(copy);
	} else if (commandName == "Select Stems" && root) {
		SaveSelection *copy = new SaveSelection(&this->selection);
		this->selection.selectStems();
		addSelectionToHistory(copy);
	} else if (commandName == "Animate") {
		if (this->timer->isActive())
			endAnimation();
		else
			startAnimation();
	} else if (commandName == "View Top") {
		this->camera.setOrientation(0.0f, 0.0f);
	} else if (commandName == "View Bottom") {
		this->camera.setOrientation(pi, 0.0f);
	} else if (commandName == "View Right") {
		this->camera.setOrientation(pi*0.5f, pi*0.5f);
	} else if (commandName == "View Left") {
		this->camera.setOrientation(pi*0.5f, -pi*0.5f);
	} else if (commandName == "View Front") {
		this->camera.setOrientation(pi*0.5f, 0.0f);
	} else if (commandName == "View Back") {
		this->camera.setOrientation(pi*0.5f, pi);
	}

	update();
	QWidget::keyPressEvent(event);
}

void Editor::exitCommand(bool changed)
{
	if (changed)
		change();
	if (this->command->isDone()) {
		this->history.add(this->command);
		this->command = nullptr;
		this->rotating = false;
	}
}

void Editor::addSelectionToHistory(SaveSelection *selection)
{
	if (selection->hasChanged()) {
		selection->setAfter();
		this->history.add(selection);
		emit selectionChanged();
		updateSelection();
		update();
	} else
		delete selection;
}

void Editor::wheelEvent(QWheelEvent *event)
{
	QPoint a = event->angleDelta();
	if (!a.isNull()) {
		this->camera.zoom(a.y()/8.0f);
		update();
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
	if (this->command) {
		exitCommand(this->command->onMousePress(event));
	} else if (event->button() == Qt::RightButton && !isAnimating()) {
		SaveSelection *selectionCopy;
		selectionCopy = new SaveSelection(&this->selection);
		Selector selector(&this->camera);
		selector.select(event, &this->mesh, &this->selection);
		if (selectionCopy->hasChanged()) {
			selectionCopy->setAfter();
			this->history.add(selectionCopy);
			emit selectionChanged();
			updateSelection();
			update();
		}
	} else if (event->button() == Qt::MiddleButton) {
		this->camera.setStartCoordinates(pos.x(), pos.y());
		if (event->modifiers() & Qt::ControlModifier)
			this->camera.setAction(Camera::Zoom);
		else if (event->modifiers() & Qt::ShiftModifier)
			this->camera.setAction(Camera::Pan);
		else
			this->camera.setAction(Camera::Rotate);
	} else if (event->button() == Qt::LeftButton && !isAnimating()) {
		selectAxis(pos.x(), pos.y());
		bool axis = this->translationAxes.getSelection();
		bool hasPoints = this->selection.hasPoints();
		bool hasFirstPoint = this->selection.hasPoint(0);
		if (hasPoints && !hasFirstPoint && axis != Axes::None) {
			MovePath *movePath = new MovePath(
				&this->selection,
				&this->translationAxes,
				&this->camera);
			Vec3 axesPosition = this->translationAxes.getPosition();
			Vec3 s = this->camera.toScreenSpace(axesPosition);
			movePath->setClickOffset(s.x - pos.x(), s.y - pos.y());
			this->command = movePath;
		}
	}
	setFocus();
}

void Editor::mouseReleaseEvent(QMouseEvent *event)
{
	if (this->command)
		exitCommand(this->command->onMouseRelease(event));
	else if (event->button() == Qt::MiddleButton)
		this->camera.setAction(Camera::None);
}

void Editor::mouseMoveEvent(QMouseEvent *event)
{
	if (this->command)
		exitCommand(this->command->onMouseMove(event));
	else {
		QPoint point = event->pos();
		bool executed;
		executed = this->camera.executeAction(point.x(), point.y());
		if (executed)
			update();
	}
}

void Editor::selectAxis(int x, int y)
{
	if (this->selection.hasPoint(0))
		return;

	float distance;
	if (this->camera.isPerspective()) {
		Vec3 cameraPosition = this->camera.getPosition();
		Vec3 axesPosition = this->translationAxes.getPosition();
		distance = pg::magnitude(cameraPosition - axesPosition);
	} else
		distance = this->camera.getDistance() * 1.25f;

	pg::Ray ray = this->camera.getRay(x, y);
	this->translationAxes.selectAxis(ray, distance);
}

void Editor::resizeGL(int width, int height)
{
	updateCamera(width, height);
	this->translationAxes.setScale(600.0f / height);
	deleteFramebuffers();
	createFramebuffers(false);
}

void Editor::updateCamera(int width, int height)
{
	float ratio = static_cast<float>(width) / static_cast<float>(height);
	this->camera.setWindowSize(width, height);
	if (this->perspective)
		this->camera.setPerspective(pi/6.0f, -0.01f, -1000.0f, ratio);
	else {
		ratio /= 2.0f;
		Vec3 near(ratio, 0.5f, 0.0f);
		Vec3 far(-ratio, -0.5f, -1000.0f);
		this->camera.setOrthographic(near, far);
	}
}

void Editor::paintGL()
{
	Mat4 projection = this->camera.getTransform();
	Vec3 position = this->camera.getPosition();

	glDepthFunc(GL_GEQUAL);
	glClearDepth(0.0f);
	glClearColor(0.22f, 0.23f, 0.24f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Paint the grid. */
	if (showingVolume())
		paintVolume(projection);
	else {
		this->staticBuffer.use();
		glUseProgram(this->shared->getShader(SharedResources::Flat));
		glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
		glDrawArrays(GL_LINES, this->segments.grid.pstart,
			this->segments.grid.pcount);
	}

	/* Paint the plant. */
	this->plantBuffer.use();
	if (isAnimating())
		updateJoints();
	if (this->shader == SharedResources::Solid)
		paintSolid(projection, position);
	else if (this->shader == SharedResources::Wireframe)
		paintWire(projection);
	else if (this->shader == SharedResources::Material)
		paintMaterial(projection, position);

	if (this->selection.hasStems() || this->selection.hasLeaves())
		paintOutline(projection);

	/* Paint path lines. */
	if (this->selection.hasStems() && !isAnimating()) {
		Geometry::Segment segment;

		glDepthFunc(GL_ALWAYS);
		glPointSize(6);

		this->pathBuffer.use();
		glUseProgram(this->shared->getShader(SharedResources::Line));
		glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
		glUniform2f(1, QWidget::width(), QWidget::height());
		segment = this->path.getLineSegment();
		GLvoid *offset = (GLvoid *)(segment.istart * sizeof(unsigned));
		glDrawElements(GL_LINE_STRIP, segment.icount,
			GL_UNSIGNED_INT, offset);

		auto texture = this->shared->getTexture(shared->DotTexture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUseProgram(this->shared->getShader(SharedResources::Point));
		glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
		segment = this->path.getPointSegment();
		glDrawArrays(GL_POINTS, segment.pstart, segment.pcount);

		if (this->selection.hasPoints())
			paintAxes(projection, position);
	} else if (this->selection.hasLeaves() && !isAnimating())
		paintAxes(projection, position);

	glFlush();
}

void Editor::paintOutline(const Mat4 &projection)
{
	if (isAnimating()) {
		auto type = SharedResources::DynamicOutline;
		glUseProgram(this->shared->getShader(type));
	} else {
		auto type = SharedResources::Outline;
		glUseProgram(this->shared->getShader(type));
	}
	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	glUniform2f(3, (GLfloat)width(), (GLfloat)height());

	glBindFramebuffer(GL_FRAMEBUFFER, this->msSilhouetteFramebuffer);
	glClearDepth(0.0f);
	glDepthFunc(GL_GEQUAL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniform1i(2, 0);
	for (size_t i = 0; i < this->selections.size(); i++) {
		size_t index = this->selections[i].indexStart;
		GLvoid *offset = (GLvoid *)(index * sizeof(unsigned));
		GLsizei size = this->selections[i].indexCount;
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, offset);
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->silhouetteFramebuffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, this->msSilhouetteFramebuffer);
	glBlitFramebuffer(0, 0, width(), height(), 0, 0, width(), height(),
		GL_COLOR_BUFFER_BIT, GL_LINEAR);

	this->staticBuffer.use();
	GLuint defaultFramebuffer = this->context()->defaultFramebufferObject();
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->silhouetteMap);
	glUniformMatrix4fv(0, 1, GL_FALSE, &pg::identity()[0][0]);
	glUniform1i(2, 2);
	GLvoid *offset = (GLvoid *)this->segments.plane.istart;
	GLsizei size = this->segments.plane.icount;
	glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, offset);

	/* Clear the alpha channel to an opaque value. */
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void Editor::paintWire(const Mat4 &projection)
{
	GLsizei vsize = this->mesh.getVertexCount();
	GLsizei isize = this->mesh.getIndexCount();
	if (isAnimating()) {
		auto type = SharedResources::DynamicWireframe;
		glUseProgram(this->shared->getShader(type));
	} else {
		auto type = SharedResources::Wireframe;
		glUseProgram(this->shared->getShader(type));
	}
	glPointSize(4);
	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	glDrawArrays(GL_POINTS, 0, vsize);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, isize, GL_UNSIGNED_INT, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Editor::paintSolid(const Mat4 &projection, const Vec3 &position)
{
	if (isAnimating()) {
		auto type = SharedResources::DynamicSolid;
		glUseProgram(this->shared->getShader(type));
	} else
		glUseProgram(this->shared->getShader(SharedResources::Solid));

	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	glUniform3f(1, position.x, position.y, position.z);
	GLsizei size = this->mesh.getIndexCount();
	glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);
}

void Editor::paintMaterial(const Mat4 &projection, const Vec3 &position)
{
	if (!this->updatedLight)
		updateLight();

	glBindFramebuffer(GL_FRAMEBUFFER, this->shadowFramebuffer);
	glViewport(0, 0, this->shadowMapSize, this->shadowMapSize);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (isAnimating()) {
		auto type = SharedResources::DynamicShadow;
		glUseProgram(this->shared->getShader(type));
	} else {
		auto type = SharedResources::Shadow;
		glUseProgram(this->shared->getShader(type));
	}
	Mat4 lightTransform = this->light.getTransform();
	Vec3 lightDirection = this->light.getDirection();
	glUniformMatrix4fv(0, 1, GL_FALSE, &lightTransform[0][0]);
	for (size_t i = 0, start = 0; i < this->mesh.getMeshCount(); i++) {
		unsigned index = this->mesh.getMaterialIndex(i);
		ShaderParams p = this->shared->getMaterial(index);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, p.getTexture(Material::Opacity));
		GLsizei size = this->mesh.getIndices(i)->size();
		GLvoid *ptr = (GLvoid *)start;
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, ptr);
		start += this->mesh.getIndices(i)->size() * sizeof(unsigned);
	}

	GLuint defaultFramebuffer = this->context()->defaultFramebufferObject();
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
	glViewport(0, 0, width(), height());
	if (isAnimating()) {
		auto type = SharedResources::DynamicMaterial;
		glUseProgram(this->shared->getShader(type));
	} else {
		auto type = SharedResources::Material;
		glUseProgram(this->shared->getShader(type));
	}
	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	Vec3 direction = this->camera.getDirection();
	glUniform3f(1, position.x, position.y, position.z);
	glUniform3f(2, direction.x, direction.y, direction.z);
	glUniform3f(5, lightDirection.x, lightDirection.y, lightDirection.z);
	glUniform1i(6, true);
	glUniform1i(8, this->camera.isPerspective());
	glUniformMatrix4fv(7, 1, GL_FALSE, &lightTransform[0][0]);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, this->shadowMap);
	for (size_t i = 0, start = 0; i < this->mesh.getMeshCount(); i++) {
		unsigned index = this->mesh.getMaterialIndex(i);
		ShaderParams p = this->shared->getMaterial(index);
		Material material = p.getMaterial();
		float shininess = material.getShininess();
		Vec3 ambient = material.getAmbient();
		glUniform3f(3, ambient.x, ambient.y, ambient.z);
		glUniform1f(4, shininess);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, p.getTexture(Material::Albedo));
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, p.getTexture(Material::Opacity));
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, p.getTexture(Material::Specular));
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, p.getTexture(Material::Normal));
		GLsizei size = this->mesh.getIndices(i)->size();
		GLvoid *ptr = (GLvoid *)start;
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, ptr);
		start += this->mesh.getIndices(i)->size() * sizeof(unsigned);
	}
}

void Editor::paintAxes(const Mat4 &projection, const Vec3 &position)
{
	float distance;
	this->translationAxes.setPosition(this->selection.getAveragePosition());
	if (this->camera.isPerspective()) {
		Vec3 diff = position - this->translationAxes.getPosition();
		distance = pg::magnitude(diff);
	} else
		distance = this->camera.getDistance() * 1.25f;

	glUseProgram(this->shared->getShader(SharedResources::Flat));
	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_GEQUAL);
	this->staticBuffer.use();
	if (this->rotating) {
		Vec3 direction = this->selection.getAverageDirectionFP();
		Mat4 transform = this->rotationAxes.getTransformation(
			distance, direction);
		transform = projection * transform;
		glUniformMatrix4fv(0, 1, GL_FALSE, &transform[0][0]);
		unsigned index = this->segments.rotation.istart;
		GLvoid *offset = (GLvoid *)(index * sizeof(unsigned));
		GLsizei size = this->segments.rotation.icount;
		glDrawElements(GL_LINE_STRIP, size, GL_UNSIGNED_INT, offset);
	} else {
		Mat4 mat = this->translationAxes.getTransformation(distance);
		mat = projection * mat;
		glUniformMatrix4fv(0, 1, GL_FALSE, &mat[0][0]);
		glDrawArrays(GL_LINES, this->segments.axesLines.pstart,
			this->segments.axesLines.pcount);
		unsigned index = this->segments.axesArrows.istart;
		GLvoid *offset = (GLvoid *)(index * sizeof(unsigned));
		GLsizei size = this->segments.axesArrows.icount;
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, offset);
	}
}

pg::Aabb createAABB(const pg::Mesh &mesh)
{
	pg::Aabb aabb = {};
	bool firstAABB = true;
	for (size_t i = 0; i < mesh.getMeshCount(); i++) {
		const vector<DVertex> *v = mesh.getVertices(i);
		if (!v->empty()) {
			pg::Aabb a = pg::createAABB(&v->at(0), v->size());
			if (!firstAABB)
				aabb = pg::combineAABB(aabb, a);
			else {
				firstAABB = false;
				aabb = a;
			}
		}
	}
	return aabb;
}

void Editor::updateLight()
{
	pg::Aabb aabb = createAABB(this->mesh);
	this->light.setOrientation(pi*0.3f, pi*0.0f);
	this->light.setDistance(0.0f);
	this->light.scaleOrthographicVolume(false);
	Mat4 transform = this->light.getView();
	Vec3 c[8] = {
		transform.apply(Vec3(aabb.a.x, aabb.a.y, aabb.a.z), 0.0f),
		transform.apply(Vec3(aabb.a.x, aabb.a.y, aabb.b.z), 0.0f),
		transform.apply(Vec3(aabb.a.x, aabb.b.y, aabb.a.z), 0.0f),
		transform.apply(Vec3(aabb.a.x, aabb.b.y, aabb.b.z), 0.0f),
		transform.apply(Vec3(aabb.b.x, aabb.a.y, aabb.a.z), 0.0f),
		transform.apply(Vec3(aabb.b.x, aabb.a.y, aabb.b.z), 0.0f),
		transform.apply(Vec3(aabb.b.x, aabb.b.y, aabb.a.z), 0.0f),
		transform.apply(Vec3(aabb.b.x, aabb.b.y, aabb.b.z), 0.0f)
	};
	aabb = pg::createAABB(c, 8);
	transform = this->light.getInverseView();
	float x = (aabb.b.x + aabb.a.x) * 0.5f;
	float y = (aabb.b.y + aabb.a.y) * 0.5f;
	float z = aabb.b.z;
	this->light.setTarget(transform.apply(Vec3(x, y, z), 0.0f));
	x = (aabb.b.x - aabb.a.x) * 0.5f;
	y = (aabb.b.y - aabb.a.y) * 0.5f;
	z = aabb.b.z - aabb.a.z;
	this->light.setOrthographic(Vec3(x, y, 0.0f), Vec3(-x, -y, -z));
	this->updatedLight = true;
}

void Editor::paintVolume(const Mat4 &projection)
{
	glUseProgram(this->shared->getShader(SharedResources::Flat));
	this->volumeBuffer.use();
	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	glDrawArrays(GL_LINES, 0, this->segments.volume.pcount);
}

void Editor::displayVolume(bool display)
{
	if (display && !this->scene.updating) {
		Geometry geometry;
		const pg::Volume *volume = this->scene.generator.getVolume();
		geometry.addVolume(volume);
		this->segments.volume = geometry.getSegment();
		makeCurrent();
		this->volumeBuffer.use();
		this->volumeBuffer.update(geometry);
		doneCurrent();
	} else
		this->segments.volume = {};
}

bool Editor::showingVolume() const
{
	return this->segments.volume.pcount > 0;
}

void Editor::updateSelection()
{
	this->selections.clear();
	auto stemInstances = this->selection.getStemInstances();
	for (auto &instance : stemInstances)
		this->selections.push_back(mesh.findStem(instance.first));
	auto leafInstances = this->selection.getLeafInstances();
	for (auto &instance : leafInstances)
		for (auto &leaf : instance.second) {
			pg::Mesh::LeafID id(instance.first, leaf);
			this->selections.push_back(this->mesh.findLeaf(id));
		}

	if (!stemInstances.empty()) {
		vector<Path::Segment> segments;
		for (auto &instance : stemInstances) {
			Stem *stem = instance.first;
			Vec3 location = stem->getLocation();
			pg::Path path = stem->getPath();
			pg::Spline spline = path.getSpline();
			Path::Segment segment;
			segment.spline = spline;
			segment.divisions = path.getDivisions();
			segment.location = location;
			segments.push_back(segment);
		}
		this->path.set(segments);

		int i = 0;
		auto instances = this->selection.getStemInstances();
		for (auto &instance : instances)
			this->path.setSelectedPoints(instance.second, i++);

		const Geometry *geometry = this->path.getGeometry();
		makeCurrent();
		this->pathBuffer.update(*geometry);
		doneCurrent();
	}
}

void Editor::change()
{
	if (!this->scene.updating) {
		this->updatedLight = false;
		if (isAnimating())
			endAnimation();
		updateBuffers();
		updateSelection();
		update();
		emit changed();
	}
}

void Editor::updateBuffers()
{
	if (!isValid())
		return;

	const Mesh &mesh = this->meshGenerator.generate();
	makeCurrent();
	this->plantBuffer.use();

	size_t capacity;
	capacity = this->plantBuffer.getCapacity(VertexBuffer::Points);
	if (mesh.getVertexCount() > capacity) {
		size_t count = mesh.getVertexCount() * 2;
		this->plantBuffer.allocatePointMemory(count);
	}
	capacity = this->plantBuffer.getCapacity(VertexBuffer::Indices);
	if (mesh.getIndexCount() > capacity) {
		size_t count = mesh.getIndexCount() * 2;
		this->plantBuffer.allocateIndexMemory(count);
	}
	int pointOffset = 0;
	int indexOffset = 0;
	for (size_t m = 0; m < mesh.getMeshCount(); m++) {
		const vector<DVertex> *v = mesh.getVertices(m);
		const vector<unsigned> *i = mesh.getIndices(m);
		this->plantBuffer.update(v->data(), pointOffset, v->size());
		this->plantBuffer.update(i->data(), indexOffset, i->size());
		pointOffset += v->size();
		indexOffset += i->size();
	}
	doneCurrent();
}

void Editor::changeWind()
{
	this->scene.animation = this->scene.wind.generate(&this->scene.plant);
	updateBuffers();
	update();
}

void Editor::animate()
{
	this->ticks++;
	int frameCount = this->scene.animation.getFrameCount()-1;
	int timeStep = this->scene.wind.getTimeStep();
	if (this->ticks >= frameCount * timeStep)
		this->ticks = 0;
	update();
}

bool Editor::isAnimating()
{
	bool animating = this->timer->isActive();
	bool hasFrames = this->scene.animation.getFrameCount() > 0;
	bool hasDuration = this->scene.wind.getDuration() > 0;
	if (!hasFrames)
		endAnimation();
	return animating && hasFrames && hasDuration;
}

void Editor::updateJoints()
{
	vector<pg::KeyFrame> frames;
	Stem *root = this->scene.plant.getRoot();
	frames = this->scene.animation.getFrame(this->ticks, root);
	size_t size = frames.size() * sizeof(pg::KeyFrame);
	if (size <= jointBuffer.getSize())
		this->jointBuffer.update(&frames[0], size);
	else
		this->jointBuffer.load(&frames[0], size);
}

void Editor::startAnimation()
{
	changeWind();
	this->ticks = 0;
	this->timer->start();
}

void Editor::endAnimation()
{
	this->ticks = -1;
	this->timer->stop();
}

void Editor::setDefaultPlant()
{
	pg::ParameterTree tree = this->scene.pattern.getParameterTree();
	pg::ParameterNode *root = tree.createRoot();
	std::random_device rd;
	pg::StemData stemData;
	stemData.seed = rd();
	stemData.radiusThreshold = 0.1f;
	stemData.fork = 0.0f;
	root->setData(stemData);
	pg::ParameterNode *node1 = tree.addChild("");
	stemData.density = 1.0f;
	stemData.densityCurve.setDefault(1);
	stemData.distance = 10.0f;
	stemData.radius = 0.9f;
	stemData.radiusThreshold = 0.03f;
	stemData.length = 60.0f;
	stemData.leaf.density = 3.0f;
	stemData.leaf.densityCurve.setDefault(1);
	stemData.leaf.distance = 3.0f;
	stemData.leaf.rotation = pi;
	node1->setData(stemData);
	pg::ParameterNode *node2 = tree.addChild("1");
	stemData.distance = 4.0f;
	stemData.length = 100.0f;
	stemData.density = 2.0f;
	stemData.radiusThreshold = 0.01f;
	stemData.angleVariation = 0.2f;
	node2->setData(stemData);
	pg::ParameterNode *node3 = tree.addChild("1.1");
	stemData.density = 0.0f;
	node3->setData(stemData);
	this->scene.pattern.setParameterTree(tree);
	this->scene.pattern.grow();
}

void Editor::load(const char *filename)
{
	this->scene.reset();
	this->shared->clearMaterials();
	if (filename) {
		try {
			std::ifstream stream(filename);
			boost::archive::text_iarchive ia(stream);
			ia >> this->scene;
			stream.close();
		} catch (boost::archive::archive_exception &) {
			this->scene.reset();
			this->shared->clearMaterials();
			this->scene.plant.setDefault();
		}
	} else
		this->scene.plant.setDefault();
	for (const Material &material : this->scene.plant.getMaterials())
		this->shared->addMaterial(ShaderParams(material));
}

void Editor::reset()
{
	displayVolume(false);
	this->selection.clear();
	this->history.clear();
	if (isValid())
		change();
	emit selectionChanged();
}

void Editor::change(QAction *action)
{
	QString text = action->text();
	if (text == "Perspective") {
		this->perspective = true;
		this->orthographicAction->setChecked(false);
		this->perspectiveAction->setChecked(true);
		updateCamera(width(), height());
	} else if (text == "Orthographic") {
		this->perspective = false;
		this->orthographicAction->setChecked(true);
		this->perspectiveAction->setChecked(false);
		updateCamera(width(), height());
	} else if (text == "Wireframe") {
		this->shader = SharedResources::Wireframe;
		this->wireframeAction->setChecked(true);
		this->solidAction->setChecked(false);
		this->materialAction->setChecked(false);
	} else if (text == "Solid") {
		this->shader = SharedResources::Solid;
		this->wireframeAction->setChecked(false);
		this->solidAction->setChecked(true);
		this->materialAction->setChecked(false);
	} else if (text == "Material") {
		this->shader = SharedResources::Material;
		this->wireframeAction->setChecked(false);
		this->solidAction->setChecked(false);
		this->materialAction->setChecked(true);
	}
	update();
}

pg::Plant *Editor::getPlant()
{
	return &this->scene.plant;
}

pg::Scene *Editor::getScene()
{
	return &this->scene;
}

Selection *Editor::getSelection()
{
	return &this->selection;
}

History *Editor::getHistory()
{
	return &this->history;
}

const pg::Mesh *Editor::getMesh()
{
	return &this->meshGenerator.getMesh();
}

void Editor::undo()
{
	if (!this->command) {
		this->history.undo();
		change();
		emit selectionChanged();
	}
}

void Editor::redo()
{
	if (!this->command) {
		this->history.redo();
		change();
		emit selectionChanged();
	}
}
