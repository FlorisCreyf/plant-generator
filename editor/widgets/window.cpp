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
	this->keymap.loadFromXMLFile("keymap.xml");

	if (argc > 1)
		this->filename = QString(argv[1]);

	this->editor = new Editor(&shared, &keymap, this);
	setCentralWidget(this->editor);

	createEditors();
	initEditor();

	this->widget.setupUi(this);

	QMenu *menu = createPopupMenu();
	menu->setTitle("Window");
	menuBar()->insertMenu(this->widget.menuHelp->menuAction(), menu);

	connect(this->widget.actionReportIssue, SIGNAL(triggered()),
		this, SLOT(reportIssue()));
	connect(&this->shared, SIGNAL(materialModified(ShaderParams)),
		this->editor, SLOT(updateMaterial(ShaderParams)));
}

void Window::createEditors()
{
	QScrollArea *scrollArea;
	auto areas = Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea;
	QDockWidget *dockWidget[5];

	dockWidget[0] = new QDockWidget(tr("Properties"), this);
	this->propertyBox = new PropertyBox(&this->shared, this->editor, this);
	dockWidget[0]->setAllowedAreas(areas);
	scrollArea = new QScrollArea();
	dockWidget[0]->setWidget(scrollArea);
	dockWidget[0]->setMinimumWidth(350);
	scrollArea->setWidget(this->propertyBox);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget[0]);

	connect(&shared, SIGNAL(materialAdded(ShaderParams)),
		this->propertyBox, SLOT(addMaterial(ShaderParams)));
	connect(&shared, SIGNAL(materialRenamed(QString, QString)),
		this->propertyBox, SLOT(renameMaterial(QString, QString)));
	connect(&shared, SIGNAL(materialRemoved(QString)),
		this->propertyBox, SLOT(removeMaterial(QString)));

	dockWidget[1] = new QDockWidget(tr("Curves"), this);
	this->curveEditor = new CurveEditor(&this->shared, &this->keymap, this);
	dockWidget[1]->setAllowedAreas(areas);
	dockWidget[1]->setWidget(this->curveEditor);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget[1]);

	this->propertyBox->bind(this->curveEditor);

	scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	dockWidget[2] = new QDockWidget(tr("Materials"), this);
	this->materialEditor = new MaterialEditor(&shared, this);
	dockWidget[2]->setAllowedAreas(areas);
	dockWidget[2]->setWidget(scrollArea);
	scrollArea->setWidget(this->materialEditor);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget[2]);

	connect(this->materialEditor->getViewer(), SIGNAL(ready()),
		this, SLOT(initMaterialEditor()));

	scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	dockWidget[3] = new QDockWidget(tr("Meshes"), this);
	this->meshEditor = new MeshEditor(&shared, this->editor, this);
	dockWidget[3]->setAllowedAreas(areas);
	dockWidget[3]->setWidget(scrollArea);
	scrollArea->setWidget(this->meshEditor);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget[3]);

	connect(this->meshEditor->getViewer(), SIGNAL(ready()),
		this, SLOT(initMeshEditor()));
	connect(this->meshEditor, SIGNAL(meshAdded(pg::Geometry)),
		this->propertyBox, SLOT(addMesh(pg::Geometry)));
	connect(this->meshEditor, SIGNAL(meshRenamed(QString, QString)),
		this->propertyBox, SLOT(renameMesh(QString, QString)));
	connect(this->meshEditor, SIGNAL(meshRemoved(QString)),
		this->propertyBox, SLOT(removeMesh(QString)));

	scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	dockWidget[4] = new QDockWidget(tr("Keys"), this);
	this->keyEditor = new KeyEditor(&keymap, this);
	dockWidget[4]->setAllowedAreas(areas);
	dockWidget[4]->setWidget(this->keyEditor);
	dockWidget[4]->setWidget(scrollArea);
	dockWidget[4]->setMinimumWidth(350);
	scrollArea->setWidget(this->keyEditor);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget[4]);

	tabifyDockWidget(dockWidget[4], dockWidget[3]);
	tabifyDockWidget(dockWidget[4], dockWidget[2]);
	tabifyDockWidget(dockWidget[4], dockWidget[1]);
	tabifyDockWidget(dockWidget[4], dockWidget[0]);
}

void Window::initEditor()
{
	if (this->filename.isEmpty())
		this->editor->load(nullptr);
	else
		this->editor->load(this->filename.toLatin1());

	objectLabel = new QLabel(this);
	fileLabel = new QLabel(this);
	commandLabel = new QLabel(this);
	statusBar()->addWidget(fileLabel, 1);
	statusBar()->addWidget(commandLabel, 1);
	statusBar()->addWidget(objectLabel, 0);
	connect(this->editor, SIGNAL(changed()), this, SLOT(updateStatus()));
	setFilename(this->filename);
}

void Window::initMeshEditor()
{
	if (!filename.isEmpty()) {
		auto meshes = this->editor->getPlant()->getLeafMeshes();
		for (auto &mesh : meshes)
			this->meshEditor->addMesh(mesh.second);
	} else
		this->meshEditor->addMesh();
}

void Window::initMaterialEditor()
{
	if (!filename.isEmpty()) {
		auto materials = this->editor->getPlant()->getMaterials();
		for (auto &material : materials)
			this->materialEditor->addMaterial(material.second);
	} else
		this->materialEditor->addMaterial();
}

void Window::updateStatus()
{
	const pg::Mesh *mesh = this->editor->getMesh();
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
	if (event->key() == Qt::Key_Z) {
		if (event->modifiers() & Qt::ControlModifier) {
			if (event->modifiers() & Qt::ShiftModifier)
				this->editor->redo();
			else
				this->editor->undo();
		}
	} else if (event->key() == Qt::Key_Y) {
		if (event->modifiers() & Qt::ControlModifier)
			this->editor->redo();
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
	this->editor->load(nullptr);
	this->materialEditor->clear();
	this->materialEditor->addMaterial();
	this->meshEditor->clear();
	this->meshEditor->addMesh();
	setFilename(tr(""));
}

void Window::openDialogBox()
{
	QString filename = QFileDialog::getOpenFileName(
		this, tr("Open File"), "", tr("Plant (*.plant)"));

	if (!filename.isNull() || !filename.isEmpty()) {
		this->meshEditor->clear();
		this->materialEditor->clear();
		this->editor->load(filename.toLatin1());
		auto materials = this->editor->getPlant()->getMaterials();
		for (auto &material : materials)
			this->materialEditor->addMaterial(material.second);
		auto meshes = this->editor->getPlant()->getLeafMeshes();
		for (auto &mesh : meshes)
			this->meshEditor->addMesh(mesh.second);
		setFilename(filename);
	}
}

void Window::saveAsDialogBox()
{
	QString filename = QFileDialog::getSaveFileName(
		this, tr("Save File"), "saved/untitled.plant",
		tr("Plant (*.plant)"));

	if (!filename.isNull() || !filename.isEmpty()) {
		std::ofstream stream(filename.toLatin1());
		if (stream.good()) {
			boost::archive::text_oarchive oa(stream);
			oa << *(this->editor->getPlant());
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
		oa << *(this->editor->getPlant());
		stream.close();
	}
}

void Window::exportDialogBox()
{
	const pg::Mesh *mesh = this->editor->getMesh();

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
