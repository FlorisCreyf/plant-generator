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

using pg::Mat4;
using pg::Vec3;
using pg::Stem;

const float pi = 3.14159265359f;

Editor::Editor(SharedResources *shared, KeyMap *keymap, QWidget *parent) :
	QOpenGLWidget(parent),
	generator(&scene.plant),
	mesh(&scene.plant),
	selection(&scene.plant)
{
	this->shared = shared;
	this->keymap = keymap;
	this->command = nullptr;
	this->perspective = true;
	this->shader = SharedResources::Solid;
	this->ticks = -1;

	Vec3 color1(0.102f, 0.212f, 0.6f);
	Vec3 color2(0.102f, 0.212f, 0.6f);
	Vec3 color3(0.1f, 1.0f, 0.4f);
	this->path.setColor(color1, color2, color3);
	setMouseTracking(true);
	setFocus();

	createToolBar();

	this->timer = new QTimer(this);
	this->timer->setInterval(33);
	connect(this->timer, &QTimer::timeout, this, &Editor::animate);
}

void Editor::createToolBar()
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
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
	createFramebuffers();
	this->shared->initialize();
	initializeBuffers();
	change();
}

void Editor::createFramebuffers()
{
	const int msaa = 4;

	glGenFramebuffers(1, &this->msSilhouetteFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, this->msSilhouetteFramebuffer);

	glGenTextures(1, &this->msSilhouetteMap);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, this->msSilhouetteMap);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaa, GL_RGB,
		width(), height(), GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE,
		GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE,
		GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE,
		GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE,
		GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D_MULTISAMPLE, this->msSilhouetteMap, 0);

	glGenFramebuffers(1, &this->silhouetteFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, this->silhouetteFramebuffer);
	glGenTextures(1, &this->silhouetteMap);
	glBindTexture(GL_TEXTURE_2D, this->silhouetteMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width(), height(), 0, GL_RGB,
		GL_UNSIGNED_BYTE, NULL);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, this->silhouetteMap, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
		Vec3(-1.0f, -1.0f, -1.0f),
		Vec3(0.0f, 0.0f, 0.0f),
		Vec3(0.0f, 0.0f, 0.0f));

	Vec3 colors[2];
	colors[0] = Vec3(0.4f, 0.4f, 0.4f);
	colors[1] = Vec3(0.4f, 0.4f, 0.4f);
	geometry.addGrid(5, colors, Vec3(0.3f, 0.3f, 0.3f));
	this->segments.grid = geometry.getSegment();
	this->segments.plane = geometry.append(plane);
	this->segments.axesLines = geometry.append(axesLines);
	this->segments.axesArrows = geometry.append(axesArrows);
	this->segments.rotation = geometry.append(rotationLines);

	this->staticBuffer.initialize(GL_STATIC_DRAW);
	this->staticBuffer.load(geometry);
	this->plantBuffer.initialize(GL_DYNAMIC_DRAW);
	this->plantBuffer.allocatePointMemory(1000);
	this->plantBuffer.allocateIndexMemory(1000);
	this->pathBuffer.initialize(GL_DYNAMIC_DRAW);
	this->pathBuffer.allocatePointMemory(100);
	this->pathBuffer.allocateIndexMemory(100);
	this->jointBuffer.initialize(GL_DYNAMIC_DRAW, 5);
}

void Editor::keyPressEvent(QKeyEvent *event)
{
	if (this->command) {
		QWidget::keyPressEvent(event);
		exitCommand(this->command->onKeyPress(event));
		return;
	}

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
				&this->selection, &this->camera, x, y);
			this->command->execute();
			change();
			emit selectionChanged();
		}
	} else if (commandName == "Add Stem") {
		size_t stemCount = this->selection.getStemInstances().size();
		Stem *root = this->scene.plant.getRoot();
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
		Stem *root = this->scene.plant.getRoot();
		bool hasStems = this->selection.hasStems();
		bool hasLeaves = this->selection.hasLeaves();
		bool containsRoot = this->selection.contains(root);
		if ((hasStems || hasLeaves) && !containsRoot)
			this->command = new MoveStem(
				&this->selection, &this->camera, x, y);
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
	} else if (commandName == "Select Stems") {
		SaveSelection *copy = new SaveSelection(&this->selection);
		this->selection.selectStems();
		addSelectionToHistory(copy);
	} if (commandName == "Animate") {
		if (this->timer->isActive())
			endAnimation();
		else
			startAnimation();
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
	}
}

void Editor::wheelEvent(QWheelEvent *event)
{
	QPoint a = event->angleDelta();
	if (!a.isNull()) {
		float y = a.y() / 8.0f;
		if (y != 0.0f) {
			this->camera.zoom(y);
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
	} else if (event->button() == Qt::MidButton) {
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
	else if (event->button() == Qt::MidButton)
		this->camera.setAction(Camera::None);
}

void Editor::mouseMoveEvent(QMouseEvent *event)
{
	if (this->command)
		exitCommand(this->command->onMouseMove(event));
	else {
		QPoint point = event->pos();
		this->camera.executeAction(point.x(), point.y());
		update();
	}
}

void Editor::selectAxis(int x, int y)
{
	if (this->selection.hasPoint(0))
		return;

	pg::Ray ray = this->camera.getRay(x, y);
	float distance;
	if (this->camera.isPerspective()) {
		Vec3 cameraPosition = this->camera.getPosition();
		Vec3 axesPosition = this->translationAxes.getPosition();
		distance = pg::magnitude(cameraPosition - axesPosition);
	} else {
		Vec3 far = this->camera.getFar();
		Vec3 near = this->camera.getNear();
		distance = pg::magnitude(far - near);
		distance /= 80.0f;
	}
	this->translationAxes.selectAxis(ray, distance);
}

void Editor::resizeGL(int width, int height)
{
	updateCamera(width, height);
	this->translationAxes.setScale(600.0f / height);
	deleteFramebuffers();
	createFramebuffers();
}

void Editor::updateCamera(int width, int height)
{
	float ratio = static_cast<float>(width) / static_cast<float>(height);
	this->camera.setWindowSize(width, height);
	if (this->perspective)
		this->camera.setPerspective(45.0f, 0.01f, 100.0f, ratio);
	else {
		ratio /= 2.0f;
		Vec3 a(-ratio, -0.5f, 0.0f);
		Vec3 b(ratio, 0.5f, 100.0f);
		this->camera.setOrthographic(a, b);
	}
}

void Editor::paintGL()
{
	Mat4 projection = this->camera.updateVP();
	Vec3 position = this->camera.getPosition();

	glClearDepth(1.0f);
	glClearColor(0.22f, 0.23f, 0.24f, 1.0);
	glDepthFunc(GL_LEQUAL);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

	/* Paint the grid. */
	this->staticBuffer.use();
	glUseProgram(this->shared->getShader(SharedResources::Flat));
	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	glDrawArrays(GL_LINES,
		this->segments.grid.pstart, this->segments.grid.pcount);

	/* Paint the plant. */
	this->plantBuffer.use();
	if (this->shader == SharedResources::Solid)
		paintSolid(projection, position);
	else if (this->shader == SharedResources::Wireframe)
		paintWire(projection);
	else if (this->shader == SharedResources::Material)
		paintMaterial(projection, position);

	if (this->selection.hasStems() || this->selection.hasLeaves())
		paintOutline(projection, position);

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
		glBindTexture(GL_TEXTURE_2D, texture);
		glUseProgram(this->shared->getShader(SharedResources::Point));
		glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);

		segment = this->path.getPointSegment();
		glDrawArrays(GL_POINTS, segment.pstart, segment.pcount);

		if (this->selection.hasPoints())
			paintAxes();
	} else if (this->selection.hasLeaves() && !isAnimating())
		paintAxes();

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glFlush();
}

void Editor::paintOutline(const Mat4 &projection, const Vec3 &position)
{
	GLuint defaultFramebuffer = this->context()->defaultFramebufferObject();
	GLsizei size = this->mesh.getIndexCount();
	GLvoid *offset;

	if (isAnimating()) {
		auto type = SharedResources::DynamicOutline;
		glUseProgram(this->shared->getShader(type));
	} else {
		auto type = SharedResources::Outline;
		glUseProgram(this->shared->getShader(type));
	}
	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	glUniform3f(1, position.x, position.y, position.z);
	glUniform2f(3, (GLfloat)width(), (GLfloat)height());

	glBindFramebuffer(GL_FRAMEBUFFER, this->msSilhouetteFramebuffer);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUniform1i(2, 0);
	for (size_t i = 0; i < this->selections.size(); i++) {
		size_t index = this->selections[i].indexStart;
		GLvoid *offset = (GLvoid *)(index * sizeof(unsigned));
		GLsizei size = this->selections[i].indexCount;
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, offset);
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->silhouetteFramebuffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, this->msSilhouetteFramebuffer);
	glDrawBuffer(GL_BACK);
	glBlitFramebuffer(0, 0, width(), height(), 0, 0, width(), height(),
		GL_COLOR_BUFFER_BIT, GL_LINEAR);

	this->staticBuffer.use();
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->silhouetteMap);
	glUniformMatrix4fv(0, 1, GL_FALSE, &pg::identity()[0][0]);
	glUniform1i(2, 2);
	offset = (GLvoid *)this->segments.plane.istart;
	size = this->segments.plane.icount;
	glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, offset);

	glActiveTexture(GL_TEXTURE0);
}

void Editor::paintWire(const Mat4 &projection)
{
	GLsizei vsize = this->mesh.getVertexCount();
	GLsizei isize = this->mesh.getIndexCount();
	if (isAnimating()) {
		auto type = SharedResources::DynamicWireframe;
		glUseProgram(this->shared->getShader(type));
		updateJoints();
	} else {
		auto type = SharedResources::Wireframe;
		glUseProgram(this->shared->getShader(type));
	}
	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	glPointSize(4);
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
		updateJoints();
	} else
		glUseProgram(this->shared->getShader(SharedResources::Solid));

	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	glUniform3f(1, position.x, position.y, position.z);
	GLsizei size = this->mesh.getIndexCount();
	glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);
}

void Editor::paintMaterial(const Mat4 &projection, const Vec3 &position)
{
	GLuint program;
	if (isAnimating()) {
		auto type = SharedResources::DynamicMaterial;
		program = this->shared->getShader(type);
		glUseProgram(program);
		updateJoints();
	} else {
		auto type = SharedResources::Material;
		program = this->shared->getShader(type);
		glUseProgram(program);
	}
	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	glUniform3f(1, position.x, position.y, position.z);

	size_t start = 0;
	for (size_t i = 0; i < this->mesh.getMeshCount(); i++) {
		unsigned index = this->mesh.getMaterialIndex(i);
		ShaderParams params = this->shared->getMaterial(index);
		pg::Material material = params.getMaterial();

		float shininess = material.getShininess();
		Vec3 ambient = material.getAmbient();
		glUniform3f(2, ambient.x, ambient.y, ambient.z);
		glUniform1f(3, shininess);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,
			params.getTexture(pg::Material::Albedo));
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D,
			params.getTexture(pg::Material::Opacity));
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D,
			params.getTexture(pg::Material::Specular));
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D,
			params.getTexture(pg::Material::Normal));

		GLsizei size = this->mesh.getIndices(i)->size();
		GLvoid *ptr = (GLvoid *)start;
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, ptr);
		start += this->mesh.getIndices(i)->size() * sizeof(unsigned);
	}
}

void Editor::paintAxes()
{
	Mat4 projection = this->camera.getVP();
	Vec3 position = this->camera.getPosition();

	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LEQUAL);

	glUseProgram(this->shared->getShader(SharedResources::Flat));
	this->staticBuffer.use();

	this->translationAxes.setPosition(this->selection.getAveragePosition());

	float distance;
	if (this->camera.isPerspective()) {
		Vec3 diff = position - this->translationAxes.getPosition();
		distance = pg::magnitude(diff);
	} else {
		Vec3 far = this->camera.getFar();
		Vec3 near = this->camera.getNear();
		distance = pg::magnitude(far - near);
		distance /= 80.0f;
	}

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
		glDrawArrays(GL_LINES,
			this->segments.axesLines.pstart,
			this->segments.axesLines.pcount);
		unsigned index = this->segments.axesArrows.istart;
		GLvoid *offset = (GLvoid *)(index * sizeof(unsigned));
		GLsizei size = this->segments.axesArrows.icount;
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, offset);
	}
}

/** Determine what regions in the buffer are the selection. */
void Editor::updateSelection()
{
	this->selections.clear();
	auto stemInstances = this->selection.getStemInstances();
	for (auto &instance : stemInstances)
		this->selections.push_back(mesh.findStem(instance.first));
	auto leafInstances = this->selection.getLeafInstances();
	for (auto &instance : leafInstances) {
		for (auto &leaf : instance.second) {
			pg::Mesh::LeafID id(instance.first, leaf);
			this->selections.push_back(this->mesh.findLeaf(id));
		}
	}

	if (!stemInstances.empty()) {
		std::vector<Path::Segment> segments;
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
	if (isAnimating())
		endAnimation();
	updateBuffers();
	updateSelection();
	update();
	emit changed();
}

void Editor::updateBuffers()
{
	if (!isValid())
		return;

	this->mesh.generate();

	makeCurrent();
	this->plantBuffer.use();

	size_t capacity;
	capacity = this->plantBuffer.getCapacity(VertexBuffer::Points);
	if (this->mesh.getVertexCount() > capacity) {
		size_t count = this->mesh.getVertexCount() * 2;
		this->plantBuffer.allocatePointMemory(count);
	}
	capacity = this->plantBuffer.getCapacity(VertexBuffer::Indices);
	if (this->mesh.getIndexCount() > capacity) {
		size_t count = this->mesh.getIndexCount() * 2;
		this->plantBuffer.allocateIndexMemory(count);
	}

	int pointOffset = 0;
	int indexOffset = 0;
	for (size_t m = 0; m < this->mesh.getMeshCount(); m++) {
		const std::vector<pg::DVertex> *v = this->mesh.getVertices(m);
		const std::vector<unsigned> *i = this->mesh.getIndices(m);
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
	std::vector<pg::KeyFrame> frames;
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

void Editor::createDefaultPlant()
{
	pg::ParameterTree tree = this->generator.getParameterTree();
	pg::ParameterRoot *root = tree.createRoot();
	std::random_device rd;
	root->setSeed(rd());
	pg::ParameterNode *node1 = tree.addChild("");
	pg::StemData data;
	data.density = 1.0f;
	data.densityCurve.setDefault(1);
	data.start = 2.0f;
	data.scale = 0.8f;
	data.length = 50.0f;
	data.radiusThreshold = 0.02f;
	data.leaf.scale = Vec3(1.0f, 1.0f, 1.0f);
	data.leaf.density = 3.0f;
	data.leaf.densityCurve.setDefault(1);
	data.leaf.distance = 3.0f;
	data.leaf.rotation = pi;
	node1->setData(data);
	pg::ParameterNode *node2 = tree.addChild("1");
	data.start = 1.0f;
	data.radiusThreshold = 0.01f;
	data.angleVariation = 0.2f;
	node2->setData(data);
	pg::ParameterNode *node3 = tree.addChild("1.1");
	data.density = 0.0f;
	node3->setData(data);
	this->generator.setParameterTree(tree);
	this->generator.grow();
	this->scene.wind.setSpeed(0.5f);
	this->scene.wind.setDirection(Vec3(0.0f, 0.0f, 1.0f));
}

void Editor::load(const char *filename)
{
	this->scene.plant.removeRoot();
	if (filename == nullptr)
		createDefaultPlant();
	else {
		std::ifstream stream(filename);
		boost::archive::text_iarchive ia(stream);
		ia >> this->scene;
		stream.close();
	}
}

void Editor::reset()
{
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

void Editor::updateMaterial(unsigned index)
{
	ShaderParams params = this->shared->getMaterial(index);
	this->scene.plant.updateMaterial(params.getMaterial(), index);
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

const pg::Mesh *Editor::getMesh()
{
	return &this->mesh;
}

void Editor::add(Command *cmd)
{
	this->history.add(cmd);
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
