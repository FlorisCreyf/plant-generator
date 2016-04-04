/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef VIEW_EDITOR_H
#define VIEW_EDITOR_H

#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <QtOpenGL/QGLWidget>

class ViewEditor : public QGLWidget
{
	Q_OBJECT

public:
	ViewEditor(QWidget *parent = 0);
	~ViewEditor();
	
protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int, int);
	void mousePressEvent(QMouseEvent *);
	void keyPressEvent(QKeyEvent *);
	void mouseMoveEvent(QMouseEvent *);

private:
	struct ShaderInfo {
		GLenum type;
		const char *filename;
	};
	
	GLuint loadShaders(ShaderInfo *info, int size);
};

#endif /* VIEW_EDITOR_H */

