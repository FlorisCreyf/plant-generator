/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef SCENE_EDITOR_H
#define SCENE_EDITOR_H

#define GL_GLEXT_PROTOTYPES

#include "treemaker.h"
#include "render_system.h"
#include "scene.h"
#include "camera.h"
#include <QtGui/QOpenGLFunctions>
#include <QOpenGLWidget>

class SceneEditor : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT;

public:
	SceneEditor(QWidget *parent = 0);
	~SceneEditor();

	void exportObject(const char *filename);
	void setRenderSystem(RenderSystem *rs);

public slots:
	void changeResolution(int i);
	void changeSections(int i);
	void changeRadius(double d);
	void changeRadiusCurve(vector<TMvec3> c);
	void changeBranchCurve(vector<TMvec3> c);
	void changeBranchDensity(double d);

signals:
	void selectionChanged(TMtree tree, int s);

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void keyPressEvent(QKeyEvent *);
	void keyReleaseEvent(QKeyEvent *);
	void mouseMoveEvent(QMouseEvent *);

private:
	bool ctrl;
	bool shift;
	bool midButton;
	int buffer;

	TMtree tree;
	Scene scene;
	Camera camera;
	RenderSystem *rs;

	void initializeTree();
	void initializeGrid();
	void selectBranch(int x, int y);
	void updateWireframe();
	void expandBuffer(GeometryComponent &g, RenderComponent &r);
	void change();
};

#endif /* SCENE_EDITOR_H */
