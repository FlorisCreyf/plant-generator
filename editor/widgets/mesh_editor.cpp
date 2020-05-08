/* Plant Genererator
 * Copyright (C) 2019  Floris Creyf
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

#include "mesh_editor.h"
#include "item_delegate.h"
#include "plant_generator/file.h"

MeshEditor::MeshEditor(
	SharedResources *shared, Editor *editor, QWidget *parent) :
	QWidget(parent)
{
	this->shared = shared;
	this->editor = editor;
	setFocusPolicy(Qt::StrongFocus);

	QVBoxLayout *columns = new QVBoxLayout(this);
	columns->setSizeConstraint(QLayout::SetMinimumSize);
	columns->setSpacing(0);
	columns->setMargin(0);

	QHBoxLayout *topRow = new QHBoxLayout();
	topRow->setSizeConstraint(QLayout::SetMinimumSize);
	topRow->setSpacing(0);
	topRow->setMargin(0);
	initTopRow(topRow);
	columns->addLayout(topRow);

	this->meshViewer = new MeshViewer(shared, this);
	this->meshViewer->setMinimumHeight(200);
	columns->addWidget(this->meshViewer);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->setMargin(5);
	layout->setSpacing(2);
	initFields(layout);
	columns->addLayout(layout);
	columns->addStretch(1);

	connect(this, SIGNAL(meshChanged(pg::Geometry)),
		this->meshViewer, SLOT(updateMesh(pg::Geometry)));
}

QSize MeshEditor::sizeHint() const
{
	return QSize(350, 200);
}

void MeshEditor::initFields(QVBoxLayout *layout)
{
	this->customButton = new QPushButton("Custom", this);
	this->customButton->setFixedHeight(22);
	this->planeButton = new QPushButton("Plane", this);
	this->planeButton->setFixedHeight(22);
	this->perpPlaneButton = new QPushButton("Perpendicular Planes", this);
	this->perpPlaneButton->setFixedHeight(22);
	this->emptyButton = new QPushButton("Empty", this);
	this->emptyButton->setFixedHeight(22);

	QString style = tr("text-align:left;padding:0 5px;");
	this->customButton->setStyleSheet(style);
	this->planeButton->setStyleSheet(style);
	this->perpPlaneButton->setStyleSheet(style);
	this->emptyButton->setStyleSheet(style);

	layout->addWidget(this->customButton);
	layout->addWidget(this->planeButton);
	layout->addWidget(this->perpPlaneButton);
	layout->addWidget(this->emptyButton);

	connect(this->customButton, SIGNAL(clicked()),
		this, SLOT(loadCustom()));
	connect(this->planeButton, SIGNAL(clicked()),
		this, SLOT(loadPlane()));
	connect(this->perpPlaneButton, SIGNAL(clicked()),
		this, SLOT(loadPerpPlane()));
	connect(this->emptyButton, SIGNAL(clicked()),
		this, SLOT(loadEmpty()));
}

void MeshEditor::initTopRow(QHBoxLayout *topRow)
{
	this->addButton = new QPushButton("+", this);
	this->addButton->setFixedHeight(22);
	this->addButton->setFixedWidth(22);

	this->removeButton = new QPushButton("-", this);
	this->removeButton->setFixedHeight(22);
	this->removeButton->setFixedWidth(22);

	this->meshBox = new QComboBox(this);
	this->meshBox->setEditable(true);
	this->meshBox->setInsertPolicy(QComboBox::InsertAtCurrent);
	this->meshBox->setItemDelegate(new ItemDelegate());

	topRow->addWidget(this->meshBox);
	topRow->addWidget(this->removeButton);
	topRow->addWidget(this->addButton);
	topRow->setAlignment(Qt::AlignTop);

	connect(this->meshBox->lineEdit(), SIGNAL(editingFinished()),
		this, SLOT(renameMesh()));
	connect(this->addButton, SIGNAL(clicked()),
		this, SLOT(addMesh()));
	connect(this->removeButton, SIGNAL(clicked()),
		this, SLOT(removeMesh()));
	connect(this->meshBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(selectMesh()));
}

void MeshEditor::clear()
{
	for (int i = 0; i < this->meshBox->count(); i++)
		emit meshRemoved(this->meshBox->itemText(i));
	this->meshBox->clear();
}

const MeshViewer *MeshEditor::getViewer() const
{
	return this->meshViewer;
}

void MeshEditor::addMesh()
{
	pg::Geometry geom;
	std::string name;
	QString qname;

	for (int i = 1; true; i++) {
		name = "Mesh " + std::to_string(i);
		qname = QString::fromStdString(name);
		if (this->meshBox->findText(qname) == -1)
			break;
	}

	geom.setName(name);
	geom.setPlane();
	addMesh(geom);
}

void MeshEditor::addMesh(pg::Geometry geom)
{
	pg::Plant *plant = this->editor->getPlant();
	QString qname = QString::fromStdString(geom.getName());
	plant->addLeafMesh(geom);
	this->meshBox->addItem(qname, QVariant((qlonglong)geom.getID()));
	this->meshBox->setCurrentIndex(this->meshBox->findText(qname));
	emit meshChanged(geom);
	emit meshAdded(geom);
}

void MeshEditor::loadCustom()
{
	if (this->meshBox->count() > 0) {
		QString filename = QFileDialog::getOpenFileName(
			this, tr("Open File"), "", tr("Wavefront OBJ (*.obj)"));

		if (!filename.isNull()) {
			pg::Plant *plant = this->editor->getPlant();
			int id = this->meshBox->currentData().toInt();
			pg::Geometry geom = plant->getLeafMesh(id);
			pg::File file;
			std::string s = filename.toStdString();
			file.importObj(s.c_str(), &geom);
			plant->addLeafMesh(geom);
			emit meshChanged(geom);
			this->editor->change();
		}
	}
}

void MeshEditor::loadPlane()
{
	if (this->meshBox->count() > 0) {
		pg::Plant *plant = editor->getPlant();
		int id = this->meshBox->currentData().toInt();
		pg::Geometry geom = plant->getLeafMesh(id);
		geom.setPlane();
		plant->addLeafMesh(geom);
		emit meshChanged(geom);
		this->editor->change();
	}
}

void MeshEditor::loadPerpPlane()
{
	if (this->meshBox->count() > 0) {
		pg::Plant *plant = this->editor->getPlant();
		int id = this->meshBox->currentData().toInt();
		pg::Geometry geom = plant->getLeafMesh(id);
		geom.setPerpendicularPlanes();
		plant->addLeafMesh(geom);
		emit meshChanged(geom);
		this->editor->change();
	}
}

void MeshEditor::loadEmpty()
{
	if (this->meshBox->count() > 0) {
		pg::Plant *plant = this->editor->getPlant();
		int id = this->meshBox->currentData().toInt();
		pg::Geometry geom = plant->getLeafMesh(id);
		geom.clear();
		plant->addLeafMesh(geom);
		emit meshChanged(geom);
		this->editor->change();
	}
}

void MeshEditor::selectMesh()
{
	pg::Plant *plant = editor->getPlant();
	this->selectedText = this->meshBox->currentText();
	int id = this->meshBox->currentData().toInt();
	if (id != 0) {
		pg::Geometry geom = plant->getLeafMesh(id);
		emit meshChanged(geom);
	}
}

void MeshEditor::renameMesh()
{
	int index = this->meshBox->currentIndex();
	QString value = this->meshBox->itemText(index);
	int id = this->meshBox->currentData().toInt();
	pg::Plant *plant = this->editor->getPlant();
	pg::Geometry geom = plant->getLeafMesh(id);
	geom.setName(value.toStdString());
	plant->addLeafMesh(geom);
	emit meshRenamed(this->selectedText, value);
	this->selectedText = value;
}

void MeshEditor::removeMesh()
{
	 if (this->meshBox->count() > 1) {
		pg::Plant *plant = this->editor->getPlant();
		int id = this->meshBox->currentData().toInt();
		QString name = this->meshBox->currentText();

		pg::Geometry geom = plant->getLeafMesh(id);
		plant->removeLeafMesh(id);

		this->meshBox->removeItem(this->meshBox->currentIndex());
		selectMesh();
		emit meshRemoved(name);
	}
}
