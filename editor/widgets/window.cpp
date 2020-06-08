/* Plant Generator
 * Copyright (C) 2016-2018  Floris Creyf
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

#include "window.h"
#include "plant_generator/file.h"
#include <boost/archive/text_iarchive.hpp>
#include <fstream>
#include <QFileDialog>

Window::Window(int argc, char **argv)
{
	this->keymap.loadFromXMLFile("keymap.xml");

	if (argc > 1)
		this->filename = QString(argv[1]);

	this->editor = new Editor(&this->shared, &this->keymap, this);
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

QDockWidget *Window::createDockWidget(
	const char *name, QWidget *widget, bool scrollbar)
{
	QDockWidget *dw = new QDockWidget(tr(name), this);
	dw->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	if (scrollbar) {
		QScrollArea *sa = new QScrollArea();
		sa->setWidgetResizable(true);
		sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		dw->setWidget(sa);
		sa->setWidget(widget);
	} else
		dw->setWidget(widget);
	return dw;
}

void Window::createEditors()
{
	QDockWidget *dw[6];

	this->propertyEditor = new PropertyEditor(
		&this->shared, this->editor, this);
	dw[0] = createDockWidget("Properties", this->propertyEditor, true);
	dw[0]->setMinimumWidth(350);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[0]);

	connect(&shared, SIGNAL(materialAdded(ShaderParams)),
		this->propertyEditor, SLOT(addMaterial(ShaderParams)));
	connect(&shared, SIGNAL(materialRenamed(QString, QString)),
		this->propertyEditor, SLOT(renameMaterial(QString, QString)));
	connect(&shared, SIGNAL(materialRemoved(QString)),
		this->propertyEditor, SLOT(removeMaterial(QString)));

	this->keyEditor = new KeyEditor(&keymap, this);
	dw[1] = createDockWidget("Keys", this->keyEditor, true);
	dw[1]->setMinimumWidth(350);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[1]);

	this->curveEditor = new CurveEditor(&this->shared, &this->keymap, this);
	dw[2] = createDockWidget("Curves", this->curveEditor, false);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[2]);
	this->propertyEditor->bind(this->curveEditor);

	this->materialEditor = new MaterialEditor(&this->shared, this);
	dw[3] = createDockWidget("Materials", this->materialEditor, true);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[3]);

	connect(this->materialEditor->getViewer(), SIGNAL(ready()),
		this, SLOT(initMaterialEditor()));

	this->meshEditor = new MeshEditor(&this->shared, this->editor, this);
	dw[4] = createDockWidget("Meshes", this->meshEditor, true);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[4]);

	this->genEditor = new GeneratorEditor(this->editor, this);
	dw[5] = createDockWidget("Generator", this->genEditor, true);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[5]);

	connect(this->meshEditor->getViewer(), SIGNAL(ready()),
		this, SLOT(initMeshEditor()));
	connect(this->meshEditor, SIGNAL(meshAdded(pg::Geometry)),
		this->propertyEditor, SLOT(addMesh(pg::Geometry)));
	connect(this->meshEditor, SIGNAL(meshRenamed(QString, QString)),
		this->propertyEditor, SLOT(renameMesh(QString, QString)));
	connect(this->meshEditor, SIGNAL(meshRemoved(QString)),
		this->propertyEditor, SLOT(removeMesh(QString)));

	tabifyDockWidget(dw[1], dw[5]);
	tabifyDockWidget(dw[1], dw[0]);
	tabifyDockWidget(dw[4], dw[3]);
	tabifyDockWidget(dw[4], dw[2]);
}

void Window::initEditor()
{
	if (this->filename.isEmpty())
		this->editor->load(nullptr);
	else
		this->editor->load(this->filename.toLatin1());

	this->objectLabel = new QLabel(this);
	this->fileLabel = new QLabel(this);
	this->commandLabel = new QLabel(this);
	statusBar()->addWidget(this->fileLabel, 1);
	statusBar()->addWidget(this->commandLabel, 1);
	statusBar()->addWidget(this->objectLabel, 0);
	connect(this->editor, SIGNAL(changed()), this, SLOT(updateStatus()));
	setFilename(this->filename);
}

void Window::initMeshEditor()
{
	if (!this->filename.isEmpty()) {
		auto meshes = this->editor->getPlant()->getLeafMeshes();
		for (auto &mesh : meshes)
			this->meshEditor->addMesh(mesh.second);
	} else
		this->meshEditor->addMesh();
}

void Window::initMaterialEditor()
{
	if (!this->filename.isEmpty()) {
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
	this->objectLabel->setText(QString::fromStdString(value));
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
		QDateTime time(QDateTime::currentDateTime());
		message.append(" | ");
		message.append(time.time().toString());
	} else {
		title = tr("Plant Generator");
		message = tr("Not Saved");
	}
	this->fileLabel->setText(message);
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
	if (this->filename.isNull() || this->filename.isEmpty())
		saveAsDialogBox();
	else {
		std::ofstream stream(this->filename.toLatin1());
		boost::archive::text_oarchive oa(stream);
		oa << *(this->editor->getPlant());
		stream.close();
		setFilename(this->filename);
	}
}

void Window::exportWavefrontDialogBox()
{
	const pg::Mesh *mesh = this->editor->getMesh();
	const pg::Plant *plant = this->editor->getPlant();

	QString filename = QFileDialog::getSaveFileName(
		this, tr("Export File"), "saved/plant.obj",
		tr("Wavefront (*.obj);;All Files (*)"));

	if (!filename.isEmpty()) {
		pg::File f;
		QByteArray b = filename.toLatin1();
		f.exportObj(b.data(), *mesh, *plant);
	}
}

void Window::exportColladaDialogBox()
{
	const pg::Mesh *mesh = this->editor->getMesh();
	const pg::Plant *plant = this->editor->getPlant();

	QString filename = QFileDialog::getSaveFileName(
		this, tr("Export File"), "saved/plant.dae",
		tr("Collada (*.dae);;All Files (*)"));

	if (!filename.isEmpty()) {
		pg::File f;
		QByteArray b = filename.toLatin1();
		f.exportDae(b.data(), *mesh, *plant);
	}
}

void Window::reportIssue()
{
	QString link = "https://github.com/FlorisCreyf/plant-generator/issues";
	QDesktopServices::openUrl(QUrl(link));
}
