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
#include "editor/file.h"
#include <boost/archive/text_iarchive.hpp>
#include <fstream>
#include <QFileDialog>

Window::Window(int argc, char **argv)
{
	keymap.loadFromXMLFile("keymap.xml");

	if (argc > 1)
		filename = QString(argv[1]);

	editor = new Editor(&shared, &keymap, this);
	this->setCentralWidget(editor);
	createEditors();
	initEditor();
	widget.setupUi(this);

	QMenu *menu = createPopupMenu();
	menu->setTitle("Window");
	menuBar()->insertMenu(widget.menuHelp->menuAction(), menu);

	connect(widget.actionReportIssue, SIGNAL(triggered()), this,
		SLOT(reportIssue()));
	connect(&shared, SIGNAL(materialModified(ShaderParams)), editor,
		SLOT(updateMaterial(ShaderParams)));
}

void Window::createEditors()
{
	QScrollArea *scrollArea;
	auto areas = Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea;
	QDockWidget *dockWidget[5];

	dockWidget[0] = new QDockWidget(tr("Properties"), this);
	propertyBox = new PropertyBox(&shared, editor, this);
	dockWidget[0]->setAllowedAreas(areas);
	scrollArea = new QScrollArea();
	dockWidget[0]->setWidget(scrollArea);
	dockWidget[0]->setMinimumWidth(350);
	scrollArea->setWidget(propertyBox);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget[0]);

	connect(&shared, SIGNAL(materialAdded(ShaderParams)), propertyBox,
		SLOT(addMaterial(ShaderParams)));
	connect(&shared, SIGNAL(materialRenamed(QString, QString)), propertyBox,
		SLOT(renameMaterial(QString, QString)));
	connect(&shared, SIGNAL(materialRemoved(QString)), propertyBox,
		SLOT(removeMaterial(QString)));

	dockWidget[1] = new QDockWidget(tr("Curves"), this);
	curveEditor = new CurveEditor(&shared, &keymap, this);
	dockWidget[1]->setAllowedAreas(areas);
	dockWidget[1]->setWidget(curveEditor);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget[1]);

	propertyBox->bind(curveEditor);

	scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	dockWidget[2] = new QDockWidget(tr("Materials"), this);
	materialEditor = new MaterialEditor(&shared, this);
	dockWidget[2]->setAllowedAreas(areas);
	dockWidget[2]->setWidget(scrollArea);
	scrollArea->setWidget(materialEditor);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget[2]);

	connect(materialEditor->getViewer(), SIGNAL(ready()), this,
		SLOT(initMaterialEditor()));

	scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	dockWidget[3] = new QDockWidget(tr("Meshes"), this);
	meshEditor = new MeshEditor(&shared, editor, this);
	dockWidget[3]->setAllowedAreas(areas);
	dockWidget[3]->setWidget(scrollArea);
	scrollArea->setWidget(meshEditor);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget[3]);

	connect(meshEditor->getViewer(), SIGNAL(ready()), this,
		SLOT(initMeshEditor()));
	connect(meshEditor, SIGNAL(meshAdded(pg::Geometry)),
		propertyBox, SLOT(addMesh(pg::Geometry)));
	connect(meshEditor, SIGNAL(meshRenamed(QString, QString)),
		propertyBox, SLOT(renameMesh(QString, QString)));
	connect(meshEditor, SIGNAL(meshRemoved(QString)), propertyBox,
		SLOT(removeMesh(QString)));

	scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	dockWidget[4] = new QDockWidget(tr("Keys"), this);
	keyEditor = new KeyEditor(&keymap, this);
	dockWidget[4]->setAllowedAreas(areas);
	dockWidget[4]->setWidget(keyEditor);
	dockWidget[4]->setWidget(scrollArea);
	dockWidget[4]->setMinimumWidth(350);
	scrollArea->setWidget(keyEditor);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget[4]);

	tabifyDockWidget(dockWidget[4], dockWidget[3]);
	tabifyDockWidget(dockWidget[4], dockWidget[2]);
	tabifyDockWidget(dockWidget[4], dockWidget[1]);
	tabifyDockWidget(dockWidget[4], dockWidget[0]);
}

void Window::initEditor()
{
	if (filename.isEmpty())
		editor->load(nullptr);
	else
		editor->load(filename.toLatin1());

	objectLabel = new QLabel(this);
	fileLabel = new QLabel(this);
	commandLabel = new QLabel(this);
	statusBar()->addWidget(fileLabel, 1);
	statusBar()->addWidget(commandLabel, 1);
	statusBar()->addWidget(objectLabel, 0);
	connect(editor, SIGNAL(changed()), this, SLOT(updateStatus()));
	setFilename(filename);
}

void Window::initMeshEditor()
{
	if (!filename.isEmpty()) {
		auto meshes = editor->getPlant()->getLeafMeshes();
		for (auto &mesh : meshes)
			meshEditor->addMesh(mesh.second);
	} else
		meshEditor->addMesh();
}

void Window::initMaterialEditor()
{
	if (!filename.isEmpty()) {
		auto materials = editor->getPlant()->getMaterials();
		for (auto &material : materials)
			materialEditor->addMaterial(material.second);
	} else
		materialEditor->addMaterial();
}

void Window::updateStatus()
{
	const pg::Mesh *mesh = editor->getMesh();
	unsigned vertices = mesh->getVertexCount();
	unsigned triangles = mesh->getIndexCount() / 3;
	unsigned materials = mesh->getMeshCount();

	std::string value;
	value += "Vertices: " + std::to_string(vertices);
	value += " | Triangles: " + std::to_string(triangles);
	value += " | Materials: " + std::to_string(materials);
	objectLabel->setText(QString::fromStdString(value));
}

void Window::Window::keyPressEvent(QKeyEvent *event)
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

void Window::setFilename(QString filename)
{
	QFileInfo fileInfo(filename);
	this->filename = filename;
	QString name = fileInfo.fileName();
	QString title = name;
	QString message = name;

	if (!name.isEmpty()) {
		title.prepend("Plant Generator — ");
		message.prepend("File: [");
		message.append("]");
	} else {
		title = tr("Plant Generator");
		message = tr("File: (Empty)");
	}
	fileLabel->setText(message);
	setWindowTitle(title);
}

void Window::newFile()
{
	editor->load(nullptr);
	materialEditor->clear();
	materialEditor->addMaterial();
	meshEditor->clear();
	meshEditor->addMesh();
	setFilename(tr(""));
}

void Window::openDialogBox()
{
	QString filename = QFileDialog::getOpenFileName(
		this, tr("Open File"), "", tr("Plant (*.plant)"));

	if (!filename.isNull() || !filename.isEmpty()) {
		meshEditor->clear();
		materialEditor->clear();
		editor->load(filename.toLatin1());
		auto materials = editor->getPlant()->getMaterials();
		for (auto &material : materials)
			materialEditor->addMaterial(material.second);
		auto meshes = editor->getPlant()->getLeafMeshes();
		for (auto &mesh : meshes)
			meshEditor->addMesh(mesh.second);
		setFilename(filename);
	}
}

void Window::saveAsDialogBox()
{
	QString filename = QFileDialog::getSaveFileName(
		this, tr("Save File"),
		"saved/untitled.plant", tr("Plant (*.plant)"));

	if (!filename.isNull() || !filename.isEmpty()) {
		std::ofstream stream(filename.toLatin1());
		if (stream.good()) {
			boost::archive::text_oarchive oa(stream);
			oa << *(editor->getPlant());
		}
		stream.close();
		setFilename(filename);
	}
}

void Window::saveDialogBox()
{
	if (filename.isNull() || filename.isEmpty())
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
	const pg::Mesh *mesh = editor->getMesh();

	QString filename = QFileDialog::getSaveFileName(
		this, tr("Export File"), "saved/plant.obj",
		tr("Wavefront OBJ (*.obj)"));

	if (!filename.isEmpty()) {
		File f;
		QByteArray b = filename.toLatin1();
		f.exportObj(b.data(), mesh->getVertices(), mesh->getIndices());
	}
}

void Window::reportIssue()
{
	QString link = "https://github.com/FlorisCreyf/plant-generator/issues";
	QDesktopServices::openUrl(QUrl(link));
}
