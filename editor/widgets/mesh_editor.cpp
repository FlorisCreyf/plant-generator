/* Plant Generator
 * Copyright (C) 2019  Floris Creyf
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

#include "mesh_editor.h"
#include "definitions.h"
#include "item_delegate.h"
#include "plant_generator/file/wavefront.h"

MeshEditor::MeshEditor(
	SharedResources *shared, Editor *editor, QWidget *parent) :
	QWidget(parent), shared(shared), editor(editor)
{
	setFocusPolicy(Qt::StrongFocus);

	this->layout = new QVBoxLayout(this);
	this->layout->setSizeConstraint(QLayout::SetMinimumSize);
	this->layout->setSpacing(0);
	this->layout->setMargin(0);

	createSelectionBar();

	this->meshViewer = new MeshViewer(shared, this);
	this->meshViewer->setMinimumHeight(200);
	this->layout->addWidget(this->meshViewer);

	QVBoxLayout *buttonLayout = new QVBoxLayout();
	buttonLayout->setMargin(UI_FORM_MARGIN);
	buttonLayout->setSpacing(UI_FORM_SPACING);
	createFields(buttonLayout);
	this->layout->addLayout(buttonLayout);
	this->layout->addStretch(1);
}

QSize MeshEditor::sizeHint() const
{
	return QSize(UI_WIDGET_WIDTH, UI_WIDGET_WIDTH);
}

void MeshEditor::createSelectionBar()
{
	QHBoxLayout *row = new QHBoxLayout();
	row->setSizeConstraint(QLayout::SetMinimumSize);
	row->setSpacing(0);
	row->setMargin(0);

	QPushButton *addButton = new QPushButton("+", this);
	addButton->setFixedHeight(UI_FIELD_HEIGHT);
	addButton->setFixedWidth(UI_FIELD_HEIGHT);

	QPushButton *removeButton = new QPushButton("-", this);
	removeButton->setFixedHeight(UI_FIELD_HEIGHT);
	removeButton->setFixedWidth(UI_FIELD_HEIGHT);

	this->selectionBox = new QComboBox(this);
	this->selectionBox->setEditable(true);
	this->selectionBox->setInsertPolicy(QComboBox::InsertAtCurrent);
	this->selectionBox->setItemDelegate(new ItemDelegate());

	row->addWidget(this->selectionBox);
	row->addWidget(removeButton);
	row->addWidget(addButton);
	row->setAlignment(Qt::AlignTop);
	this->layout->addLayout(row);

	connect(this->selectionBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(select()));
	connect(this->selectionBox->lineEdit(), SIGNAL(editingFinished()),
		this, SLOT(rename()));
	connect(addButton, SIGNAL(clicked()), this, SLOT(add()));
	connect(removeButton, SIGNAL(clicked()), this, SLOT(remove()));
}

void MeshEditor::createFields(QVBoxLayout *layout)
{
	QPushButton *button[5];
	button[0] = new QPushButton("Custom", this);
	button[0]->setFixedHeight(UI_FIELD_HEIGHT);
	button[1] = new QPushButton("Plane", this);
	button[1]->setFixedHeight(UI_FIELD_HEIGHT);
	button[2] = new QPushButton("Perpendicular Planes", this);
	button[2]->setFixedHeight(UI_FIELD_HEIGHT);
	button[3] = new QPushButton("Pyramid");
	button[3]->setFixedHeight(UI_FIELD_HEIGHT);
	button[4] = new QPushButton("Empty", this);
	button[4]->setFixedHeight(UI_FIELD_HEIGHT);

	layout->addWidget(button[0]);
	layout->addWidget(button[1]);
	layout->addWidget(button[2]);
	layout->addWidget(button[3]);
	layout->addWidget(button[4]);

	connect(button[0], SIGNAL(clicked()), this, SLOT(loadCustom()));
	connect(button[1], SIGNAL(clicked()), this, SLOT(loadPlane()));
	connect(button[2], SIGNAL(clicked()), this, SLOT(loadPerpPlane()));
	connect(button[3], SIGNAL(clicked()), this, SLOT(loadPyramid()));
	connect(button[4], SIGNAL(clicked()), this, SLOT(loadEmpty()));
}

void MeshEditor::clear()
{
	pg::Plant *plant = this->editor->getPlant();
	int count = this->selectionBox->count();
	while (count > 0) {
		plant->removeLeafMesh(count-1);
		emit meshRemoved(count-1);
		count--;
	}
	this->selectionBox->clear();
}

const MeshViewer *MeshEditor::getViewer() const
{
	return this->meshViewer;
}

void MeshEditor::reset()
{
	this->selectionBox->blockSignals(true);
	this->selectionBox->clear();
	auto meshes = this->editor->getPlant()->getLeafMeshes();
	for (pg::Geometry mesh : meshes) {
		QString name = QString::fromStdString(mesh.getName());
		this->selectionBox->addItem(name);
		emit meshAdded(mesh);
	}
	this->selectionBox->blockSignals(false);
	select();
}

void MeshEditor::add()
{
	pg::Geometry geom;
	std::string name;
	QString qname;

	for (int i = 1; true; i++) {
		name = "Mesh " + std::to_string(i);
		qname = QString::fromStdString(name);
		if (this->selectionBox->findText(qname) == -1)
			break;
	}

	geom.setName(name);
	geom.setPlane();
	add(geom);
}

void MeshEditor::add(pg::Geometry geom)
{
	pg::Plant *plant = this->editor->getPlant();
	plant->addLeafMesh(geom);
	this->selectionBox->blockSignals(true);
	QString name = QString::fromStdString(geom.getName());
	this->selectionBox->addItem(name);
	this->selectionBox->setCurrentIndex(this->selectionBox->findText(name));
	this->selectionBox->blockSignals(false);
	this->meshViewer->updateMesh(geom);
	emit meshAdded(geom);
}

void MeshEditor::loadCustom()
{
	if (this->selectionBox->count() > 0) {
		QString filename = QFileDialog::getOpenFileName(
			this, "Open File", "", "Wavefront OBJ (*.obj)");

		if (!filename.isNull()) {
			pg::Plant *plant = this->editor->getPlant();
			unsigned index = this->selectionBox->currentIndex();
			pg::Geometry geom = plant->getLeafMesh(index);
			pg::Wavefront obj;
			std::string s = filename.toStdString();
			obj.importFile(s.c_str(), &geom);
			modify(geom, index);
		}
	}
}

void MeshEditor::loadPlane()
{
	if (this->selectionBox->count() > 0) {
		pg::Plant *plant = editor->getPlant();
		unsigned index = this->selectionBox->currentIndex();
		pg::Geometry geom = plant->getLeafMesh(index);
		geom.setPlane();
		modify(geom, index);
	}
}

void MeshEditor::loadPerpPlane()
{
	if (this->selectionBox->count() > 0) {
		pg::Plant *plant = this->editor->getPlant();
		unsigned index = this->selectionBox->currentIndex();
		pg::Geometry geom = plant->getLeafMesh(index);
		geom.setPerpendicularPlanes();
		modify(geom, index);
	}
}

void MeshEditor::loadPyramid()
{
	if (this->selectionBox->count() > 0) {
		pg::Plant *plant = this->editor->getPlant();
		unsigned index = this->selectionBox->currentIndex();
		pg::Geometry geom = plant->getLeafMesh(index);
		geom.setPyramid();
		modify(geom, index);
	}
}

void MeshEditor::loadEmpty()
{
	if (this->selectionBox->count() > 0) {
		pg::Plant *plant = this->editor->getPlant();
		unsigned index = this->selectionBox->currentIndex();
		pg::Geometry geom = plant->getLeafMesh(index);
		geom.clear();
		modify(geom, index);
	}
}

void MeshEditor::modify(pg::Geometry &geom, unsigned index)
{
	pg::Plant *plant = this->editor->getPlant();
	plant->updateLeafMesh(geom, index);
	this->meshViewer->updateMesh(geom);
	emit meshModified(geom, index);
	this->editor->change();
}

void MeshEditor::select()
{
	if (this->selectionBox->count()) {
		pg::Plant *plant = editor->getPlant();
		unsigned index = this->selectionBox->currentIndex();
		pg::Geometry geom = plant->getLeafMesh(index);
		this->meshViewer->updateMesh(geom);
	}
}

void MeshEditor::rename()
{
	unsigned index = this->selectionBox->currentIndex();
	QString name = this->selectionBox->itemText(index);
	pg::Plant *plant = this->editor->getPlant();
	pg::Geometry geom = plant->getLeafMesh(index);
	geom.setName(name.toStdString());
	plant->updateLeafMesh(geom, index);
	emit meshModified(geom, index);
}

void MeshEditor::remove()
{
	 if (this->selectionBox->count() > 1) {
		unsigned index = this->selectionBox->currentIndex();
		QString name = this->selectionBox->currentText();
		pg::Plant *plant = this->editor->getPlant();
		plant->removeLeafMesh(index);
		this->selectionBox->removeItem(index);
		select();
		this->editor->change();
		emit meshRemoved(index);
	}
}
