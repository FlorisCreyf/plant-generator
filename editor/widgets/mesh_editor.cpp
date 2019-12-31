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
#include "editor/file.h"

MeshEditor::MeshEditor(SharedResources *shared, Editor *editor,
	QWidget *parent) : QWidget(parent)
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

	meshViewer = new MeshViewer(shared, this);
	meshViewer->setMinimumHeight(200);
	columns->addWidget(meshViewer);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->setMargin(10);
	layout->setSpacing(2);
	initFields(layout);
	columns->addLayout(layout);
	columns->addStretch(1);

	connect(this, SIGNAL(meshChanged(pg::Geometry)),
		meshViewer, SLOT(updateMesh(pg::Geometry)));
}

QSize MeshEditor::sizeHint() const
{
	return QSize(350, 200);
}

void MeshEditor::initFields(QVBoxLayout *layout)
{
	customButton = new QPushButton("Custom", this);
	customButton->setFixedHeight(22);
	planeButton = new QPushButton("Plane", this);
	planeButton->setFixedHeight(22);
	perpPlaneButton = new QPushButton("Perpendicular Planes", this);
	perpPlaneButton->setFixedHeight(22);
	emptyButton = new QPushButton("Empty", this);
	emptyButton->setFixedHeight(22);


	QString style = tr("text-align:left;padding:0 5px;");
	customButton->setStyleSheet(style);
	planeButton->setStyleSheet(style);
	perpPlaneButton->setStyleSheet(style);
	emptyButton->setStyleSheet(style);

	layout->addWidget(customButton);
	layout->addWidget(planeButton);
	layout->addWidget(perpPlaneButton);
	layout->addWidget(emptyButton);

	connect(customButton, SIGNAL(clicked()), this, SLOT(loadCustom()));
	connect(planeButton, SIGNAL(clicked()), this, SLOT(loadPlane()));
	connect(perpPlaneButton, SIGNAL(clicked()),
		this, SLOT(loadPerpPlane()));
	connect(emptyButton, SIGNAL(clicked()), this, SLOT(loadEmpty()));
}

void MeshEditor::initTopRow(QHBoxLayout *topRow)
{
	addButton = new QPushButton("+", this);
	addButton->setFixedHeight(22);
	addButton->setFixedWidth(22);

	removeButton = new QPushButton("-", this);
	removeButton->setFixedHeight(22);
	removeButton->setFixedWidth(22);

	meshBox = new QComboBox(this);
	meshBox->setEditable(true);
	meshBox->setInsertPolicy(QComboBox::NoInsert);
	meshBox->setItemDelegate(new ItemDelegate());

	topRow->addWidget(meshBox);
	topRow->addWidget(removeButton);
	topRow->addWidget(addButton);
	topRow->setAlignment(Qt::AlignTop);

	connect(meshBox, SIGNAL(editTextChanged(const QString &)),
		this, SLOT(renameMesh(const QString &)));
	connect(addButton, SIGNAL(clicked()), this, SLOT(addMesh()));
	connect(removeButton, SIGNAL(clicked()), this, SLOT(removeMesh()));
	connect(meshBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(selectMesh()));
}

void MeshEditor::clear()
{
	for (int i = 0; i < meshBox->count(); i++)
		emit meshRemoved(meshBox->itemText(i));
	meshBox->clear();
}

const MeshViewer *MeshEditor::getViewer() const
{
	return meshViewer;
}

void MeshEditor::addMesh()
{
	pg::Geometry geom;
	std::string name;
	QString qname;

	for (int i = 1; true; i++) {
		name = "Mesh " + std::to_string(i);
		qname = QString::fromStdString(name);
		if (meshBox->findText(qname) == -1)
			break;
	}

	geom.setName(name);
	geom.setPlane();
	addMesh(geom);
}

void MeshEditor::addMesh(pg::Geometry geom)
{
	pg::Plant *plant = editor->getPlant();
	QString qname = QString::fromStdString(geom.getName());
	plant->addLeafMesh(geom);
	meshBox->addItem(qname, QVariant((qlonglong)geom.getID()));
	meshBox->setCurrentIndex(meshBox->findText(qname));
	emit meshChanged(geom);
	emit meshAdded(geom);
}

void MeshEditor::loadCustom()
{
	if (meshBox->count() > 0) {
		QString filename = QFileDialog::getOpenFileName(
			this, tr("Open File"), "", tr("Wavefront OBJ (*.obj)"));

		if (!filename.isNull()) {
			pg::Plant *plant = editor->getPlant();
			int id = meshBox->currentData().toInt();
			pg::Geometry geom = plant->getLeafMesh(id);
			File file;
			std::string s = filename.toStdString();
			file.importObj(s.c_str(), &geom);
			plant->addLeafMesh(geom);
			emit meshChanged(geom);
			editor->change();
		}
	}
}

void MeshEditor::loadPlane()
{
	if (meshBox->count() > 0) {
		pg::Plant *plant = editor->getPlant();
		int id = meshBox->currentData().toInt();
		pg::Geometry geom = plant->getLeafMesh(id);
		geom.setPlane();
		plant->addLeafMesh(geom);
		emit meshChanged(geom);
		editor->change();
	}
}

void MeshEditor::loadPerpPlane()
{
	if (meshBox->count() > 0) {
		pg::Plant *plant = editor->getPlant();
		int id = meshBox->currentData().toInt();
		pg::Geometry geom = plant->getLeafMesh(id);
		geom.setPerpendicularPlanes();
		plant->addLeafMesh(geom);
		emit meshChanged(geom);
		editor->change();
	}
}

void MeshEditor::loadEmpty()
{
	if (meshBox->count() > 0) {
		pg::Plant *plant = editor->getPlant();
		int id = meshBox->currentData().toInt();
		pg::Geometry geom = plant->getLeafMesh(id);
		geom.clear();
		plant->addLeafMesh(geom);
		emit meshChanged(geom);
		editor->change();
	}
}

void MeshEditor::selectMesh()
{
	pg::Plant *plant = editor->getPlant();
	int id = meshBox->currentData().toInt();
	pg::Geometry geom = plant->getLeafMesh(id);
	emit meshChanged(geom);
}

void MeshEditor::renameMesh(const QString &text)
{
	if (!text.isEmpty() && meshBox->findText(text) == -1) {
		QString before = meshBox->itemText(meshBox->currentIndex());
		int id = meshBox->currentData().toInt();
		pg::Plant *plant = editor->getPlant();
		pg::Geometry geom = plant->getLeafMesh(id);
		geom.setName(text.toStdString());
		plant->addLeafMesh(geom);
		meshBox->setItemText(meshBox->currentIndex(), text);
		emit meshRenamed(before, text);
	}
}

void MeshEditor::removeMesh()
{
	 if (meshBox->count() > 1) {
		pg::Plant *plant = editor->getPlant();
 		int id = meshBox->currentData().toInt();
		QString name = meshBox->currentText();

		pg::Geometry geom = plant->getLeafMesh(id);
		plant->removeLeafMesh(id);

		meshBox->removeItem(meshBox->currentIndex());
		selectMesh();
		emit meshRemoved(name);
	 }
}
