/* Plant Generator
 * Copyright (C) 2019  Floris Creyf
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

#include "material_viewer.h"
#include "plant_generator/math/vec3.h"
#include "plant_generator/math/mat4.h"

#undef near
#undef far

using pg::Vec3;
using pg::Mat4;
using pg::Material;

MaterialViewer::MaterialViewer(SharedResources *shared, QWidget *parent) :
	QOpenGLWidget(parent), shared(shared), materialIndex(0)
{
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);
	this->camera.setTarget(Vec3(0.5f, 0.5f, 0.0f));
	this->camera.setOrientation(0.0f, 0.0f);
	this->camera.setDistance(0.6f);
	this->camera.setPanSpeed(0.004f);
	this->camera.setZoom(0.01f, 0.3f, 2.0f);
}

QSize MaterialViewer::sizeHint() const
{
	return QSize(width(), 150);
}

void MaterialViewer::updateMaterial(unsigned materialIndex)
{
	this->materialIndex = materialIndex;
	update();
}

void MaterialViewer::createInterface()
{
	Geometry plane;
	Vec3 a(1.0f, 0.0f, 0.0f);
	Vec3 b(0.0f, 1.0f, 0.0f);
	Vec3 c(0.0f, 0.0f, 0.0f);
	Vec3 normal(0.0f, 0.0f, 1.0f);
	Vec3 tangent(0.0f, 1.0f, 0.0f);
	plane.addPlane(a, b, c, normal, tangent);
	this->planeSegment = plane.getSegment();
	this->buffer.update(plane);
}

void MaterialViewer::initializeGL()
{
	initializeOpenGLFunctions();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_ALWAYS);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_PRIMITIVE_RESTART);
	this->buffer.initialize(GL_DYNAMIC_DRAW);
	this->buffer.allocatePointMemory(20);
	this->buffer.allocateIndexMemory(10);
	glPrimitiveRestartIndex(Geometry::primitiveReset);
	createInterface();
	this->shared->initialize();
}

void MaterialViewer::paintGL()
{
	glClearColor(0.32f, 0.32f, 0.32f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ShaderParams params = this->shared->getMaterial(this->materialIndex);
	float aspect = params.getMaterial().getRatio();
	Material material = params.getMaterial();
	float shininess = material.getShininess();
	Vec3 ambient = material.getAmbient();
	Mat4 scale;
	if (aspect < 1.0f)
		scale = pg::scale(Vec3(1.0f, 1.0f/aspect, 1.0f));
	else
		scale = pg::scale(Vec3(aspect, 1.0f, 1.0f));
	Mat4 projection = scale * this->camera.getTransform();
	Vec3 position = this->camera.getPosition();
	Vec3 direction = this->camera.getDirection();

	this->buffer.use();
	glUseProgram(this->shared->getShader(SharedResources::Material));
	glUniformMatrix4fv(0, 1, GL_FALSE, &projection[0][0]);
	glUniform3f(1, position.x, position.y, position.z);
	glUniform3f(2, direction.x, direction.y, direction.z);
	glUniform3f(3, ambient.x, ambient.y, ambient.z);
	glUniform1f(4, shininess);
	glUniform3f(5, direction.x, direction.y, direction.z);
	glUniform1i(6, false);
	glUniform1i(8, this->camera.isPerspective());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, params.getTexture(Material::Albedo));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, params.getTexture(Material::Opacity));
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, params.getTexture(Material::Specular));
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, params.getTexture(Material::Normal));

	auto size = sizeof(unsigned);
	GLvoid *start = (GLvoid *)(this->planeSegment.istart * size);
	GLsizei count = this->planeSegment.icount;
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, start);

	glFlush();
}

void MaterialViewer::resizeGL(int width, int height)
{
	float ratio = static_cast<float>(width) / static_cast<float>(height);
	Vec3 near(ratio, 1.0f, 0.0f);
	Vec3 far(-ratio, -1.0f, -100.0f);
	this->camera.setOrthographic(near, far);
	this->camera.setWindowSize(width, height);
}

void MaterialViewer::mousePressEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	if (event->button() == Qt::MiddleButton) {
		this->camera.setStartCoordinates(p.x(), p.y());
		if (event->modifiers() & Qt::ControlModifier)
			this->camera.setAction(Camera::Zoom);
		else if (event->modifiers() & Qt::ShiftModifier)
			this->camera.setAction(Camera::Pan);
	}
}

void MaterialViewer::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MiddleButton)
		this->camera.setAction(Camera::None);
}

void MaterialViewer::mouseMoveEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	this->camera.executeAction(p.x(), p.y());
	update();
}
