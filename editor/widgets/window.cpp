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
#include "form.h"
#include "plant_generator/file/collada.h"
#include "plant_generator/file/wavefront.h"
#include <fstream>
#include <QFileDialog>
#include <boost/archive/text_iarchive.hpp>

Window::Window(int argc, char **argv)
{
	this->keymap.loadFromXMLFile("keymap.xml");
	if (argc > 1)
		this->filename = QString(argv[1]);

	this->objectLabel = new QLabel(this);
	this->fileLabel = new QLabel(this);
	statusBar()->addWidget(this->fileLabel, 1);
	statusBar()->addWidget(this->objectLabel, 0);
	setFilename(this->filename);

	this->editor = new Editor(&this->shared, &this->keymap, this);
	setCentralWidget(this->editor);
	createEditors();
	initEditor();
	this->widget.setupUi(this);

	QMenu *menu = createPopupMenu();
	menu->setTitle("Window");
	menuBar()->insertMenu(this->widget.menuHelp->menuAction(), menu);

	connect(this->editor, &Editor::changed, this, &Window::updateStatus);
	connect(this->widget.actionReportIssue, &QAction::triggered,
		this, &Window::reportIssue);
}

QDockWidget *Window::createDockWidget(
	const char *name, QWidget *widget, bool scrollbar)
{
	QDockWidget *dw = new QDockWidget(name, this);
	dw->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dw->setMinimumWidth(UI_WIDGET_WIDTH);
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
	QDockWidget *dw[7];

	this->propertyEditor = new PropertyEditor(
		&this->shared, this->editor, this);
	dw[0] = createDockWidget("Properties", this->propertyEditor, true);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[0]);
	connect(&shared, &SharedResources::materialAdded,
		this->propertyEditor, &PropertyEditor::addMaterial);
	connect(&shared, &SharedResources::materialModified,
		this->propertyEditor, &PropertyEditor::updateMaterials);
	connect(&shared, &SharedResources::materialRemoved,
		this->propertyEditor, &PropertyEditor::removeMaterial);

	this->keyEditor = new KeyEditor(&keymap, this);
	dw[1] = createDockWidget("Key Map", this->keyEditor, true);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[1]);

	this->pCurveEditor = new PropertyCurveEditor(
		&this->shared, &this->keymap, this->editor, this);
	dw[2] = createDockWidget("Curves", this->pCurveEditor, false);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[2]);
	connect(this->pCurveEditor, &PropertyCurveEditor::curveAdded,
		this->propertyEditor, &PropertyEditor::addCurve);
	connect(this->pCurveEditor, &PropertyCurveEditor::curveModified,
		this->propertyEditor, &PropertyEditor::updateCurve);
	connect(this->pCurveEditor, &PropertyCurveEditor::curveRemoved,
		this->propertyEditor, &PropertyEditor::removeCurve);

	this->materialEditor = new MaterialEditor(
		&this->shared, this->editor, this);
	dw[3] = createDockWidget("Materials", this->materialEditor, true);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[3]);

	this->meshEditor = new MeshEditor(&this->shared, this->editor, this);
	dw[4] = createDockWidget("Meshes", this->meshEditor, true);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[4]);
	connect(this->meshEditor, &MeshEditor::meshAdded,
		this->propertyEditor, &PropertyEditor::addMesh);
	connect(this->meshEditor, &MeshEditor::meshModified,
		this->propertyEditor, &PropertyEditor::updateMesh);
	connect(this->meshEditor, &MeshEditor::meshRemoved,
		this->propertyEditor, &PropertyEditor::removeMesh);

	dw[5] = createDockWidget("Generator", createGeneratorEditor(), true);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[5]);

	this->gCurveEditor = new GeneratorCurveEditor(
		&this->shared, &this->keymap, this->editor, this);
	dw[6] = createDockWidget("Generator Curves", this->gCurveEditor, false);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[6]);
	connect(this->generatorEditor, &GeneratorEditor::parameterTreeModified,
		this->gCurveEditor, &GeneratorCurveEditor::setFields);

	tabifyDockWidget(dw[1], dw[5]);
	tabifyDockWidget(dw[1], dw[0]);
	tabifyDockWidget(dw[4], dw[3]);
	tabifyDockWidget(dw[4], dw[6]);
	tabifyDockWidget(dw[4], dw[2]);
}

QWidget *Window::createGeneratorEditor()
{
	this->generatorEditor = new GeneratorEditor(this->editor, this);
	this->windEditor = new WindEditor(this->editor, this);
	QWidget *widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setSizeConstraint(QLayout::SetMinimumSize);
	layout->setSpacing(0);
	layout->setMargin(0);
	layout->addWidget(this->generatorEditor);
	layout->addWidget(this->windEditor);
	layout->addStretch(1);
	return widget;
}

void Window::initEditor()
{
	if (this->filename.isEmpty())
		newFile();
	else {
		this->propertyEditor->clearOptions();
		this->editor->load(this->filename.toLatin1());
		this->pCurveEditor->reset();
		this->meshEditor->reset();
		this->materialEditor->reset();
		this->editor->reset();
	}
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

void Window::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Z) {
		if (event->modifiers() & Qt::ControlModifier) {
			if (event->modifiers() & Qt::ShiftModifier) {
				this->editor->redo();
				this->pCurveEditor->select();
			} else {
				this->editor->undo();
				this->pCurveEditor->select();
			}
		}
	} else if (event->key() == Qt::Key_Y) {
		if (event->modifiers() & Qt::ControlModifier) {
			this->editor->redo();
			this->pCurveEditor->select();
		}
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
		title = "Plant Generator";
		message = "Not Saved";
	}
	this->fileLabel->setText(message);
	setWindowTitle(title);
}

void Window::newFile()
{
	this->propertyEditor->clearOptions();
	this->pCurveEditor->clear();
	this->pCurveEditor->add();
	this->materialEditor->clear();
	this->materialEditor->addEmpty();
	this->meshEditor->clear();
	this->meshEditor->addEmpty();
	this->editor->load(nullptr);
	this->windEditor->setFields();
	this->editor->reset();
	setFilename("");
}

void Window::openDialogBox()
{
	QString filename = QFileDialog::getOpenFileName(
		this, "Open File", "", "Plant (*.plant)");

	if (!filename.isNull() || !filename.isEmpty()) {
		this->propertyEditor->clearOptions();
		this->editor->load(filename.toLatin1());
		this->windEditor->setFields();
		this->pCurveEditor->reset();
		this->meshEditor->reset();
		this->materialEditor->reset();
		this->editor->reset();
		setFilename(filename);
	}
}

void Window::saveAsDialogBox()
{
	QString filename = QFileDialog::getSaveFileName(
		this, "Save File", "saved/untitled.plant", "Plant (*.plant)");

	if (!filename.isNull() || !filename.isEmpty()) {
		std::ofstream stream(filename.toLatin1());
		if (stream.good()) {
			boost::archive::text_oarchive oa(stream);
			oa << *(this->editor->getScene());
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
		oa << *(this->editor->getScene());
		stream.close();
		setFilename(this->filename);
	}
}

void Window::exportWavefrontDialogBox()
{
	const pg::Mesh *mesh = this->editor->getMesh();
	const pg::Plant *plant = this->editor->getPlant();
	QString filename = QFileDialog::getSaveFileName(
		this, "Export File", "saved/plant.obj",
		"Wavefront (*.obj);;All Files (*)");

	if (!filename.isEmpty()) {
		pg::Wavefront obj;
		QByteArray array = filename.toLatin1();
		obj.exportFile(array.data(), *mesh, *plant);
	}
}

void Window::exportColladaDialogBox()
{
	this->editor->changeWind();
	const pg::Mesh *mesh = this->editor->getMesh();
	const pg::Scene *scene = this->editor->getScene();
	QString filename = QFileDialog::getSaveFileName(
		this, "Export File", "saved/plant.dae",
		"Collada (*.dae);;All Files (*)");

	if (!filename.isEmpty()) {
		pg::Collada dae;
		QByteArray array = filename.toLatin1();
		dae.exportFile(array.data(), *mesh, *scene);
	}
}

void Window::reportIssue()
{
	QString link = "https://github.com/FlorisCreyf/plant-generator/issues";
	QDesktopServices::openUrl(QUrl(link));
}
