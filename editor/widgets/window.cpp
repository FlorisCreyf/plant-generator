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

#include "window.h"
#include "../file_exporter.h"
#include <boost/archive/text_iarchive.hpp>
#include <fstream>
#include <QFileDialog>

Window::Window()
{
	editor = new Editor(&shared, this);
	this->setCentralWidget(editor);
	createPropertyBox();
	createCurveEditor();
	propertyBox->bind(editor, curveEditor);
	widget.setupUi(this);

	QMenu *menu = createPopupMenu();
	menu->setTitle("Window");
	menuBar()->insertMenu(widget.menuHelp->menuAction(), menu);
	connect(widget.actionReportIssue, SIGNAL(triggered()), this,
		SLOT(reportIssue()));
}

void Window::createPropertyBox()
{
	auto areas = Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea;
	QScrollArea *scrollArea = new QScrollArea();
	QDockWidget *dockWidget = new QDockWidget(tr("Properties"), this);
	propertyBox = new PropertyBox(&shared, this);
	dockWidget->setAllowedAreas(areas);
        dockWidget->setWidget(scrollArea);
	dockWidget->setMinimumWidth(200);
	scrollArea->setWidget(propertyBox);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget);
}

void Window::createCurveEditor()
{
	auto areas = Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea;
	QDockWidget *dockWidget = new QDockWidget(tr("Curve"), this);
	curveEditor = new CurveEditor(&shared, this);
	dockWidget->setAllowedAreas(areas);
	dockWidget->setWidget(curveEditor);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget);
}

void Window::keyPressEvent(QKeyEvent *event)
{
	if (!editor->isExecutingAction()) {
		switch (event->key()) {
		case Qt::Key_Z:
			if (event->modifiers() & Qt::ControlModifier) {
				if (event->modifiers() & Qt::ShiftModifier)
					redo();
				else
					undo();
			}
			break;
		case Qt::Key_Y:
			if (event->modifiers() & Qt::ControlModifier)
				redo();
			break;
		}
	}

	QWidget::keyPressEvent(event);
}

void Window::newFile()
{
	editor->load(nullptr);
	this->setWindowTitle("Plant Generator");
}

void Window::openDialogBox()
{
	QString filename;
	filename = QFileDialog::getOpenFileName(this, tr("Open File"), "",
		tr("Plant (*.plant)"));
	if (!filename.isNull()) {
		editor->load(filename.toLatin1());
		this->filename = filename;
		this->setWindowTitle(filename.prepend("Plant Generator — "));
	}
}

void Window::saveAsDialogBox()
{
	QString filename;
	filename = QFileDialog::getSaveFileName(this, tr("Save File"),
		"saved/untitled.plant", tr("Plant (*.plant)"));
	if (!filename.isNull()) {
		std::ofstream stream(filename.toLatin1());
		if (stream.good()) {
			boost::archive::text_oarchive oa(stream);
			oa << *(editor->getPlant());
		}
		stream.close();
		this->filename = filename;
		this->setWindowTitle(filename.prepend("Plant Generator — "));
	}
}

void Window::saveDialogBox()
{
	if (filename.isNull())
		saveAsDialogBox();
	else {
		std::ofstream stream(filename.toLatin1());
		boost::archive::text_oarchive oa(stream);
		oa << *(editor->getPlant());
		stream.close();
	}
}

void Window::exportDialogBox()
{
	QString filename;
	filename = QFileDialog::getSaveFileName(this, tr("Export File"),
		"saved/plant.obj", tr("Wavefront (*.obj)"));

	const pg::Mesh *mesh = editor->getMesh();
	FileExporter f;
	f.setVertices(mesh->getVertices()->data(), mesh->getVertices()->size());
	f.setIndices(mesh->getIndices()->data(), mesh->getIndices()->size());
	f.exportObj(filename.toLatin1());
}

void Window::reportIssue()
{
	QString link = "https://github.com/FlorisCreyf/plant-generator/issues";
	QDesktopServices::openUrl(QUrl(link));
}

void Window::undo()
{
	if (!editor->isExecutingAction())
		editor->undo();
}

void Window::redo()
{
	if (!editor->isExecutingAction())
		editor->redo();
}
