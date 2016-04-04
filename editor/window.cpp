/* 
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "window.h"
#include <QFileDialog>

Window::Window()
{
	widget.setupUi(this);
}

Window::~Window()
{

}

void Window::openDialogBox()
{
	QString filename;
	filename = QFileDialog::getOpenFileName(this, tr("Open File"), "",
		tr("BlueTree (*.tree)"));
}

void Window::saveDialogBox()
{
	QString filename;
	filename = QFileDialog::getSaveFileName(this, tr("Save File"),
		"untitled.tree", tr("BlueTree (*.tree)"));
}

void Window::exportDialogBox()
{
	QString filename;
	filename = QFileDialog::getSaveFileName(this, tr("Export File"),
		"untitled.obj", tr("Wavefront (*.obj)"));
}

