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
	connect(this->editor, &Editor::changed, this, &Window::updateStatus);
	setCentralWidget(this->editor);
	createEditors();
	initEditor();

	this->widget.setupUi(this);
	connect(this->widget.actionReportIssue, &QAction::triggered,
		this, &Window::reportIssue);

	QMenu *menu = createPopupMenu();
	menu->setTitle("Window");
	menuBar()->insertMenu(this->widget.menuHelp->menuAction(), menu);
}

QDockWidget *Window::createDW(const char *name, QWidget *widget, bool scroll)
{
	QDockWidget *dw = new QDockWidget(name, this);
	dw->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dw->setMinimumWidth(UI_WIDGET_WIDTH);
	if (scroll) {
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
	QDockWidget *dw[3];

	this->propertyEditor = new PropertyEditor(&this->shared, &this->keymap,
		this->editor, this);
	dw[0] = createDW("Properties", this->propertyEditor, true);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[0]);
	this->keyEditor = new KeyEditor(&keymap, this);
	dw[1] = createDW("Key Map", this->keyEditor, true);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[1]);
	this->generatorEditor = new GeneratorEditor(&this->shared,
		&this->keymap, this->editor, this);
	dw[2] = createDW("Generator", this->generatorEditor, true);
	addDockWidget(static_cast<Qt::DockWidgetArea>(1), dw[2]);

	tabifyDockWidget(dw[1], dw[2]);
	tabifyDockWidget(dw[1], dw[0]);
}

void Window::initEditor()
{
	if (this->filename.isEmpty())
		newFile();
	else {
		this->propertyEditor->clear();
		this->editor->load(this->filename.toLatin1());
		this->editor->reset();
		this->propertyEditor->populate();
	}
}

void Window::newFile()
{
	this->propertyEditor->clear();
	this->editor->load(nullptr);
	this->editor->reset();
	this->propertyEditor->populate();
	setFilename("");
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
		title = "Plant Generator";
		message = "Not Saved";
	}
	this->fileLabel->setText(message);
	setWindowTitle(title);
}

void Window::openDialogBox()
{
	QString filename = QFileDialog::getOpenFileName(this, "Open File", "",
		"Plant (*.plant)");
	if (!filename.isNull() || !filename.isEmpty()) {
		setFilename(filename);
		initEditor();
	}
}

void Window::saveAsDialogBox()
{
	QString filename = QFileDialog::getSaveFileName(this, "Save File",
		"saved/untitled.plant", "Plant (*.plant)");
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
	QString filename = QFileDialog::getSaveFileName(this, "Export File",
		"saved/plant.obj", "Wavefront (*.obj);;All Files (*)");
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
	QString filename = QFileDialog::getSaveFileName(this, "Export File",
		"saved/plant.dae", "Collada (*.dae);;All Files (*)");
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
