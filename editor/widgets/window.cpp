/* Plant Genererator
 * Copyright (C) 2016-2017  Floris Creyf
 *
 * TreeMaker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TreeMaker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "window.h"
#include "../file_exporter.h"
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

void Window::openDialogBox()
{
	QString filename;
	filename = QFileDialog::getOpenFileName(this, tr("Open File"), "",
		tr("Plant (*.pg)"));
}

void Window::saveDialogBox()
{
	QString filename;
	filename = QFileDialog::getSaveFileName(this, tr("Save File"),
		"saved/untitled.pg", tr("Plant (*.pg)"));
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
	QString link = "https://github.com/FlorisCreyf/treemaker/issues";
	QDesktopServices::openUrl(QUrl(link));
}
