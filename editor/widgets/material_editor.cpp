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

#include "material_editor.h"
#include "plant_generator/math/math.h"

using pg::Vec3;
using pg::Mat4;

MaterialEditor::MaterialEditor(SharedResources *shared, QWidget *parent) :
	QWidget(parent)
{
	this->shared = shared;
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

	materialViewer = new MaterialViewer(shared, this);
	columns->addWidget(materialViewer);

	QFormLayout *form = new QFormLayout();
	form->setMargin(10);
	form->setSpacing(3);
	initFields(form);
	columns->addLayout(form);
	columns->addStretch(1);

	connect(this, SIGNAL(materialChanged(ShaderParams)), materialViewer,
		SLOT(updateMaterial(ShaderParams)));
	connect(materialViewer, SIGNAL(ready()), this, SLOT(addMaterial()));
}

void MaterialEditor::initFields(QFormLayout *form)
{
	QHBoxLayout *diffuseLayout = new QHBoxLayout();
	diffuseBox = new QLineEdit(this);
	diffuseBox->setFixedHeight(22);
	diffuseBox->setReadOnly(true);
	addDiffuseButton = new QPushButton("+", this);
	addDiffuseButton->setFixedWidth(22);
	addDiffuseButton->setFixedHeight(22);
	removeDiffuseButton = new QPushButton("-", this);
	removeDiffuseButton->setFixedWidth(22);
	removeDiffuseButton->setFixedHeight(22);

	diffuseLayout->setSpacing(0);
	diffuseLayout->addWidget(diffuseBox);
	diffuseLayout->addWidget(removeDiffuseButton);
	diffuseLayout->addWidget(addDiffuseButton);
	form->addRow(tr("Diffuse Map"), diffuseLayout);

	connect(addDiffuseButton, SIGNAL(clicked()), this,
		SLOT(openDiffuseFile()));
	connect(removeDiffuseButton, SIGNAL(clicked()), this,
		SLOT(removeDiffuseFile()));
}

void MaterialEditor::initTopRow(QHBoxLayout *topRow)
{
	addButton = new QPushButton("+", this);
	addButton->setFixedHeight(22);
	addButton->setFixedWidth(22);

	removeButton = new QPushButton("-", this);
	removeButton->setFixedHeight(22);
	removeButton->setFixedWidth(22);

	materialBox = new QComboBox(this);
	materialBox->setEditable(true);
	materialBox->setInsertPolicy(QComboBox::NoInsert);
	materialBox->setItemDelegate(new ItemDelegate());

	topRow->addWidget(materialBox);;
	topRow->addWidget(removeButton);
	topRow->addWidget(addButton);
	topRow->setAlignment(Qt::AlignTop);

	connect(materialBox, SIGNAL(editTextChanged(const QString &)), this,
		SLOT(renameMaterial(const QString &)));
	connect(addButton, SIGNAL(clicked()), this, SLOT(addMaterial()));
	connect(removeButton, SIGNAL(clicked()), this, SLOT(removeMaterial()));
	connect(materialBox, SIGNAL(currentIndexChanged(int)), this,
		SLOT(selectMaterial()));
}

void MaterialEditor::addMaterial(pg::Material material)
{
	ShaderParams params(material);
	QString qname = QString::fromStdString(params.getName());
	shared->addMaterial(params);
	materialBox->addItem(qname, QVariant(params.getId()));
	materialBox->setCurrentIndex(materialBox->count() - 1);

	if (!material.getTexture().empty()) {
		std::string filename = material.getTexture();
		QString qfilename = QString::fromStdString(filename);
		diffuseBox->setText(qfilename);
		params.loadTexture(0, qfilename);
	}

	emit materialChanged(params);
}

void MaterialEditor::addMaterial()
{
	ShaderParams params;
	std::string name;
	QString qname;
	GLuint defaultTex = shared->getTexture(SharedResources::DefaultTexture);

	for (int i = 1; true; i++) {
		name = "Material " + std::to_string(i);
		qname = QString::fromStdString(name);
		if (materialBox->findText(qname) == -1)
			break;
	}

	params.setName(name);
	params.setDefaultTexture(0, defaultTex);
	shared->addMaterial(params);
	diffuseBox->setText(tr(""));
	materialBox->addItem(qname, QVariant(params.getId()));
	materialBox->setCurrentIndex(materialBox->findText(qname));
	emit materialChanged(params);
}

void MaterialEditor::clear()
{
	shared->clearMaterials();
	materialBox->clear();
	diffuseBox->clear();
}

void MaterialEditor::renameMaterial(const QString &text)
{
	if (!text.isEmpty() && materialBox->findText(text) == -1) {
		int id = materialBox->currentData().toInt();
		ShaderParams params = shared->getMaterial(id);
		params.setName(text.toStdString());
		shared->addMaterial(params);
		materialBox->setItemText(materialBox->currentIndex(), text);
	}
}

void MaterialEditor::selectMaterial()
{
	int id = materialBox->currentData().toInt();
	ShaderParams params = shared->getMaterial(id);
	std::string filename = params.getMaterial().getTexture();
	QString qfilename = QString::fromStdString(filename);
	diffuseBox->setText(qfilename);
	emit materialChanged(params);
}

void MaterialEditor::removeMaterial()
{
	if (materialBox->count() > 1) {
		int id = materialBox->currentData().toInt();
		shared->removeMaterial(id);
		materialBox->removeItem(materialBox->currentIndex());
		selectMaterial();
	}
}

void MaterialEditor::openDiffuseFile()
{
	int index = materialBox->currentIndex();
	unsigned id = materialBox->itemData(index).toInt();
	ShaderParams params = shared->getMaterial(id);
	QFileDialog dialog(this, tr("Open File"));

	dialog.exec();
	QString filename = dialog.selectedFiles().first();

	if (params.loadTexture(0, filename)) {
		shared->addMaterial(params);
		diffuseBox->setText(filename);
		emit materialChanged(params);
	}
}

void MaterialEditor::removeDiffuseFile()
{
	int index = materialBox->currentIndex();
	unsigned id = materialBox->itemData(index).toInt();
	ShaderParams params = shared->getMaterial(id);
	params.removeTexture(0);
	diffuseBox->clear();
	emit materialChanged(params);
}
