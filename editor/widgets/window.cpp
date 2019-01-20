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
	createEditors();
	propertyBox->bind(editor, curveEditor);
	widget.setupUi(this);

	QMenu *menu = createPopupMenu();
	menu->setTitle("Window");
	menuBar()->insertMenu(widget.menuHelp->menuAction(), menu);
	connect(widget.actionReportIssue, SIGNAL(triggered()), this,
		SLOT(reportIssue()));
	connect(&shared, SIGNAL(materialModified(ShaderParams)), editor,
		SLOT(updateMaterial(ShaderParams)));
}

void Window::createPropertyBox()
{
	auto areas = Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea;
	QScrollArea *scrollArea = new QScrollArea();
	QDockWidget *dockWidget = new QDockWidget(tr("Property Editor"), this);
	propertyBox = new PropertyBox(&shared, this);
	dockWidget->setAllowedAreas(areas);
        dockWidget->setWidget(scrollArea);
	dockWidget->setMinimumWidth(350);
	scrollArea->setWidget(propertyBox);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget);

	connect(&shared, SIGNAL(materialAdded(ShaderParams)), propertyBox,
		SLOT(addMaterial(ShaderParams)));
	connect(&shared, SIGNAL(materialRenamed(QString, QString)), propertyBox,
		SLOT(renameMaterial(QString, QString)));
	connect(&shared, SIGNAL(materialRemoved(QString)), propertyBox,
		SLOT(removeMaterial(QString)));
}

void Window::createEditors()
{
	auto areas = Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea;
	QDockWidget *dockWidget[2];

	dockWidget[0] = new QDockWidget(tr("Curve Editor"), this);
	curveEditor = new CurveEditor(&shared, this);
	dockWidget[0]->setAllowedAreas(areas);
	dockWidget[0]->setWidget(curveEditor);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget[0]);

	dockWidget[1] = new QDockWidget(tr("Material Editor"), this);
	materialEditor = new MaterialEditor(&shared, this);
	dockWidget[1]->setAllowedAreas(areas);
	dockWidget[1]->setWidget(materialEditor);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget[1]);

	tabifyDockWidget(dockWidget[0], dockWidget[1]);
}

void Window::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Z:
		if (event->modifiers() & Qt::ControlModifier) {
			if (event->modifiers() & Qt::ShiftModifier)
				editor->redo();
			else
				editor->undo();
		}
		break;
	case Qt::Key_Y:
		if (event->modifiers() & Qt::ControlModifier)
			editor->redo();
		break;
	}

	QWidget::keyPressEvent(event);
}

void Window::newFile()
{
	editor->load(nullptr);
	materialEditor->clear();
	materialEditor->addMaterial();
	this->setWindowTitle("Plant Generator");
}

void Window::openDialogBox()
{
	QString filename;
	filename = QFileDialog::getOpenFileName(this, tr("Open File"), "",
		tr("Plant (*.plant)"));
	if (!filename.isNull()) {
		materialEditor->clear();
		editor->load(filename.toLatin1());
		auto materials = editor->getPlant()->getMaterials();
		for (auto &material : materials)
			materialEditor->addMaterial(material.second);
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
	FileExporter f;
	const pg::Mesh *mesh = editor->getMesh();

	QString filename;
	filename = QFileDialog::getSaveFileName(this, tr("Export File"),
		"saved/plant.obj", tr("Wavefront (*.obj)"));

	if (!filename.isEmpty()) {
		QByteArray b = filename.toLatin1();
		f.exportObj(b.data(), mesh->getVertices(), mesh->getIndices());
	}
}

void Window::reportIssue()
{
	QString link = "https://github.com/FlorisCreyf/plant-generator/issues";
	QDesktopServices::openUrl(QUrl(link));
}
