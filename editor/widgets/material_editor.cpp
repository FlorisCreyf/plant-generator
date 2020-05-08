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
#include "item_delegate.h"

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

	this->materialViewer = new MaterialViewer(shared, this);
	this->materialViewer->setMinimumHeight(200);
	columns->addWidget(this->materialViewer);

	QFormLayout *form = new QFormLayout();
	form->setMargin(5);
	form->setSpacing(2);
	initFields(form);
	columns->addLayout(form);
	columns->addStretch(1);

	connect(this, SIGNAL(materialChanged(ShaderParams)),
		this->materialViewer, SLOT(updateMaterial(ShaderParams)));
}

QSize MaterialEditor::sizeHint() const
{
	return QSize(350, 200);
}

void MaterialEditor::initFields(QFormLayout *form)
{
	QHBoxLayout *diffuseLayout = new QHBoxLayout();
	this->diffuseBox = new QLineEdit(this);
	this->diffuseBox->setFixedHeight(22);
	this->diffuseBox->setReadOnly(true);
	this->addDiffuseButton = new QPushButton("+", this);
	this->addDiffuseButton->setFixedWidth(22);
	this->addDiffuseButton->setFixedHeight(22);
	this->removeDiffuseButton = new QPushButton("-", this);
	this->removeDiffuseButton->setFixedWidth(22);
	this->removeDiffuseButton->setFixedHeight(22);

	QWidget *sizeWidget = new QWidget();
	sizeWidget->setLayout(diffuseLayout);
	diffuseLayout->setSpacing(0);
	diffuseLayout->setMargin(0);
	diffuseLayout->addWidget(this->diffuseBox);
	diffuseLayout->addWidget(this->removeDiffuseButton);
	diffuseLayout->addWidget(this->addDiffuseButton);
	form->addRow(tr("Diffuse"), sizeWidget);

	form->setFormAlignment(Qt::AlignRight | Qt::AlignTop);
	form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	form->setLabelAlignment(Qt::AlignRight | Qt::AlignCenter);

	connect(this->addDiffuseButton, SIGNAL(clicked()),
		this, SLOT(openDiffuseFile()));
	connect(this->removeDiffuseButton, SIGNAL(clicked()),
		this, SLOT(removeDiffuseFile()));
}

void MaterialEditor::initTopRow(QHBoxLayout *topRow)
{
	this->addButton = new QPushButton("+", this);
	this->addButton->setFixedHeight(22);
	this->addButton->setFixedWidth(22);

	this->removeButton = new QPushButton("-", this);
	this->removeButton->setFixedHeight(22);
	this->removeButton->setFixedWidth(22);

	this->materialBox = new QComboBox(this);
	this->materialBox->setEditable(true);
	this->materialBox->setInsertPolicy(QComboBox::InsertAtCurrent);
	this->materialBox->setItemDelegate(new ItemDelegate());

	topRow->addWidget(this->materialBox);;
	topRow->addWidget(this->removeButton);
	topRow->addWidget(this->addButton);
	topRow->setAlignment(Qt::AlignTop);

	connect(this->materialBox->lineEdit(), SIGNAL(editingFinished()),
		this, SLOT(renameMaterial()));
	connect(this->addButton, SIGNAL(clicked()), this, SLOT(addMaterial()));
	connect(this->removeButton, SIGNAL(clicked()),
		this, SLOT(removeMaterial()));
	connect(this->materialBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(selectMaterial()));
}

const MaterialViewer *MaterialEditor::getViewer() const
{
	return this->materialViewer;
}

/** Add an existing material to the material list. */
void MaterialEditor::addMaterial(pg::Material material)
{
	ShaderParams params(material);
	QString qname = QString::fromStdString(params.getName());
	this->shared->addMaterial(params);
	this->materialBox->addItem(qname, QVariant((qlonglong)params.getID()));
	this->materialBox->setCurrentIndex(this->materialBox->count() - 1);

	if (!material.getTexture().empty()) {
		std::string filename = material.getTexture();
		QString qfilename = QString::fromStdString(filename);
		this->diffuseBox->setText(qfilename);
		params.loadTexture(0, qfilename);
	}

	emit materialChanged(params);
}

/** Add an empty material with a unique name to the material list. */
void MaterialEditor::addMaterial()
{
	ShaderParams params;
	std::string name;
	QString qname;
	GLuint defaultTex = this->shared->getTexture(
		SharedResources::DefaultTexture);

	for (int i = 1; true; i++) {
		name = "Material " + std::to_string(i);
		qname = QString::fromStdString(name);
		if (this->materialBox->findText(qname) == -1)
			break;
	}

	params.setName(name);
	params.setDefaultTexture(0, defaultTex);
	this->shared->addMaterial(params);
	this->diffuseBox->setText(tr(""));
	this->materialBox->addItem(qname, QVariant((qlonglong)params.getID()));
	this->materialBox->setCurrentIndex(this->materialBox->findText(qname));
	emit materialChanged(params);
}

void MaterialEditor::clear()
{
	this->shared->clearMaterials();
	this->materialBox->clear();
	this->diffuseBox->clear();
}

void MaterialEditor::renameMaterial()
{
	int index = this->materialBox->currentIndex();
	QString value = this->materialBox->itemText(index);
	int id = this->materialBox->currentData().toInt();
	ShaderParams params = this->shared->getMaterial(id);
	params.setName(value.toStdString());
	this->shared->addMaterial(params);
}

void MaterialEditor::selectMaterial()
{
	int id = this->materialBox->currentData().toInt();
	ShaderParams params = this->shared->getMaterial(id);
	std::string filename = params.getMaterial().getTexture();
	QString qfilename = QString::fromStdString(filename);
	this->diffuseBox->setText(qfilename);
	emit materialChanged(params);
}

void MaterialEditor::removeMaterial()
{
	if (this->materialBox->count() > 1) {
		int id = this->materialBox->currentData().toInt();
		this->shared->removeMaterial(id);
		this->materialBox->removeItem(
			this->materialBox->currentIndex());
		selectMaterial();
	}
}

void MaterialEditor::openDiffuseFile()
{
	int index = this->materialBox->currentIndex();
	long id = this->materialBox->itemData(index).toInt();
	ShaderParams params = this->shared->getMaterial(id);
	QFileDialog dialog(this, tr("Open File"));

	dialog.exec();
	QString filename = dialog.selectedFiles().first();

	if (params.loadTexture(0, filename)) {
		this->shared->addMaterial(params);
		this->diffuseBox->setText(filename);
		emit materialChanged(params);
	}
}

void MaterialEditor::removeDiffuseFile()
{
	int index = this->materialBox->currentIndex();
	long id = this->materialBox->itemData(index).toInt();
	ShaderParams params = this->shared->getMaterial(id);
	params.removeTexture(0);
	this->diffuseBox->clear();
	emit materialChanged(params);
}
