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
	widget.properties->bind(widget.editor, widget.curve);
	widget.editor->setRenderSystem(&r);

	QMenu *menu = createPopupMenu();
	menu->setTitle("Window");
	menuBar()->insertMenu(widget.menuHelp->menuAction(), menu);

	connect(widget.actionReportIssue, SIGNAL(triggered()), this,
			SLOT(reportIssue()));
}

void Window::openDialogBox()
{
	QString filename;
	filename = QFileDialog::getOpenFileName(this, tr("Open File"), "",
		tr("TreeMaker (*.tree)"));
}

void Window::saveDialogBox()
{
	QString filename;
	filename = QFileDialog::getSaveFileName(this, tr("Save File"),
		"untitled.tree", tr("TreeMaker (*.tree)"));
}

void Window::exportDialogBox()
{
	QString filename;
	filename = QFileDialog::getSaveFileName(this, tr("Export File"),
		"untitled.obj", tr("Wavefront (*.obj)"));
	QByteArray b = filename.toLatin1();
	widget.editor->exportObject(b.data());
}

void Window::reportIssue()
{
	QString link = "https://github.com/FlorisCreyf/treemaker/issues";
	QDesktopServices::openUrl(QUrl(link));
}
