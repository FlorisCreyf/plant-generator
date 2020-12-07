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

#include "material_editor.h"
#include "form.h"
#include "widgets.h"

using pg::Material;

MaterialEditor::MaterialEditor(
	SharedResources *shared, Editor *editor, QWidget *parent) :
	QWidget(parent),
	editor(editor),
	shared(shared),
	materialViewer(new MaterialViewer(shared, this)),
	layout(new QVBoxLayout(this)),
	selectionBox(new ComboBox(this))
{
	this->layout->setSizeConstraint(QLayout::SetMinimumSize);
	this->layout->setSpacing(0);
	this->layout->setMargin(0);

	createSelectionBar();

	this->materialViewer->setMinimumHeight(200);
	this->layout->addWidget(this->materialViewer);

	QFormLayout *form = new QFormLayout();
	form->setMargin(UI_FORM_MARGIN);
	form->setSpacing(UI_FORM_SPACING);
	initFields(form);
	this->layout->addLayout(form);
	this->layout->addStretch(1);
}

QSize MaterialEditor::sizeHint() const
{
	return QSize(UI_WIDGET_WIDTH, UI_WIDGET_WIDTH);
}

void MaterialEditor::createSelectionBar()
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

	this->selectionBox->setEditable(true);
	this->selectionBox->setInsertPolicy(ComboBox::InsertAtCurrent);
	this->selectionBox->setItemDelegate(new ItemDelegate());

	row->addWidget(this->selectionBox);
	row->addWidget(removeButton);
	row->addWidget(addButton);
	row->setAlignment(Qt::AlignTop);
	this->layout->addLayout(row);

	connect(this->selectionBox,
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, &MaterialEditor::select);
	connect(this->selectionBox->lineEdit(), &QLineEdit::editingFinished,
		this, &MaterialEditor::rename);
	connect(addButton, &QPushButton::clicked,
		this, &MaterialEditor::addEmpty);
	connect(removeButton, &QPushButton::clicked,
		this, &MaterialEditor::remove);
}

void MaterialEditor::initFields(QFormLayout *form)
{
	QWidget *fileFields[Material::MapQuantity];
	for (int i = 0; i < Material::MapQuantity; i++) {
		QHBoxLayout *layout = new QHBoxLayout();
		this->fileField[i] = new QLineEdit(this);
		this->fileField[i]->setFixedHeight(UI_FIELD_HEIGHT);
		this->fileField[i]->setReadOnly(true);
		this->removeButton[i] = new QPushButton("-", this);
		this->removeButton[i]->setFixedWidth(UI_FIELD_HEIGHT);
		this->removeButton[i]->setFixedHeight(UI_FIELD_HEIGHT);
		this->addButton[i] = new QPushButton("+", this);
		this->addButton[i]->setFixedWidth(UI_FIELD_HEIGHT);
		this->addButton[i]->setFixedHeight(UI_FIELD_HEIGHT);
		fileFields[i] = new QWidget();
		fileFields[i]->setLayout(layout);
		layout->setSpacing(0);
		layout->setMargin(0);
		layout->addWidget(this->fileField[i]);
		layout->addWidget(this->removeButton[i]);
		layout->addWidget(this->addButton[i]);

		connect(this->addButton[i], &QPushButton::clicked,
			this, [this, i]() {this->openFile(i);});
		connect(this->removeButton[i], &QPushButton::clicked,
			this, [this, i]() {this->removeFile(i);});
	}
	form->addRow("Albedo", fileFields[Material::Albedo]);
	form->addRow("Opacity", fileFields[Material::Opacity]);
	form->addRow("Normal", fileFields[Material::Normal]);
	form->addRow("Specular", fileFields[Material::Specular]);

	for (int i = 0; i < FieldQuantity; i++) {
		this->fields[i] = new DoubleSpinBox(this);
		this->fields[i]->setSingleStep(0.001);
		this->fields[i]->setDecimals(3);

		connect(this->fields[i],
			QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			this, &MaterialEditor::change);
	}
	form->addRow("Shininess", this->fields[Shininess]);
	this->fields[Shininess]->setSingleStep(1);
	this->fields[Shininess]->setDecimals(1);
	form->addRow("Ambient.R", this->fields[AmbientR]);
	form->addRow("Ambient.G", this->fields[AmbientG]);
	form->addRow("Ambient.B", this->fields[AmbientB]);

	setFormLayout(form);
}

const MaterialViewer *MaterialEditor::getViewer() const
{
	return this->materialViewer;
}

void loadMaterial(ShaderParams &params, Material &material)
{
	QString file;
	for (int i = 0; i < Material::MapQuantity; i++) {
		if (!material.getTexture(i).empty()) {
			file = QString::fromStdString(material.getTexture(i));
			params.loadTexture(i, file);
		}
	}
}

void MaterialEditor::reset()
{
	this->shared->clearMaterials();
	this->selectionBox->clear();
	clearFields();

	auto materials = this->editor->getPlant()->getMaterials();
	for (pg::Material material : materials) {
		ShaderParams params(material);
		QString name = QString::fromStdString(params.getName());
		loadMaterial(params, material);
		this->shared->addMaterial(params);
		this->selectionBox->addItem(name);
	}

	select(this->selectionBox->currentIndex());
}

void MaterialEditor::add(pg::Material material)
{
	ShaderParams params(material);
	QString name = QString::fromStdString(params.getName());
	this->selectionBox->addItem(name);
	this->selectionBox->setCurrentIndex(this->selectionBox->count() - 1);
	loadMaterial(params, material);
	this->shared->addMaterial(params);
	this->editor->getPlant()->addMaterial(params.getMaterial());
	select(this->selectionBox->currentIndex());
}

void MaterialEditor::addEmpty()
{
	ShaderParams params;
	std::string name;
	QString qname;

	for (int i = 1; true; i++) {
		name = "Material " + std::to_string(i);
		qname = QString::fromStdString(name);
		if (this->selectionBox->findText(qname) == -1)
			break;
	}

	params.setName(name);
	unsigned index = this->shared->addMaterial(params);
	this->editor->getPlant()->addMaterial(params.getMaterial());
	this->materialViewer->updateMaterial(index);
	this->selectionBox->addItem(qname);
	index = this->selectionBox->findText(qname);
	this->selectionBox->setCurrentIndex(index);
	clearFields();
}

void MaterialEditor::clear()
{
	pg::Plant *plant = this->editor->getPlant();
	int count = this->selectionBox->count();
	while (count > 0) {
		plant->removeMaterial(count-1);
		count--;
	}
	this->selectionBox->clear();
	this->shared->clearMaterials();
	clearFields();
}

void MaterialEditor::rename()
{
	unsigned index = this->selectionBox->currentIndex();
	QString value = this->selectionBox->itemText(index);
	ShaderParams params = this->shared->getMaterial(index);
	params.setName(value.toStdString());
	update(params, index);
}

QString filterFilename(QString filename)
{
	return filename.split("/").back();
}

void MaterialEditor::fillFields(const Material &material)
{
	for (int i = 0; i < Material::MapQuantity; i++) {
		std::string filename = material.getTexture(i);
		QString qfilename = QString::fromStdString(filename);
		this->fileField[i]->setText(filterFilename(qfilename));
	}
	this->fields[Shininess]->setValue(material.getShininess());
	this->fields[AmbientR]->setValue(material.getAmbient().x);
	this->fields[AmbientG]->setValue(material.getAmbient().y);
	this->fields[AmbientB]->setValue(material.getAmbient().z);
}

void MaterialEditor::change()
{
	unsigned index = this->selectionBox->currentIndex();
	ShaderParams params = this->shared->getMaterial(index);
	Material material = params.getMaterial();
	material.setShininess(this->fields[Shininess]->value());
	material.setAmbient(pg::Vec3(
		this->fields[AmbientR]->value(),
		this->fields[AmbientG]->value(),
		this->fields[AmbientB]->value()));
	params.swapMaterial(material);
	update(params, index);
}

void MaterialEditor::select(int index)
{
	if (this->selectionBox->count()) {
		ShaderParams params = this->shared->getMaterial(index);
		fillFields(params.getMaterial());
		this->materialViewer->updateMaterial(index);
	}
}

void MaterialEditor::remove()
{
	if (this->selectionBox->count() > 1) {
		unsigned index = this->selectionBox->currentIndex();
		this->selectionBox->removeItem(index);
		this->shared->removeMaterial(index);
		this->editor->getPlant()->removeMaterial(index);
		this->editor->change();
		select(this->selectionBox->currentIndex());
	}
}

void MaterialEditor::clearFields()
{
	for (int i = 0; i < Material::MapQuantity; i++)
		this->fileField[i]->clear();
}

void MaterialEditor::openFile(int index)
{
	int selection = this->selectionBox->currentIndex();
	ShaderParams params = this->shared->getMaterial(selection);
	QString filename = QFileDialog::getOpenFileName(this, "Open File", "",
		"All Files (*);;PNG (*.png);;JPEG (*.jpg);;SVG (*.svg)");

	if (!filename.isNull() || !filename.isEmpty()) {
		this->fileField[index]->setText(filterFilename(filename));
		if (params.loadTexture(index, filename))
			update(params, selection);
	}
}

void MaterialEditor::removeFile(int index)
{
	int selection = this->selectionBox->currentIndex();
	ShaderParams params = this->shared->getMaterial(selection);
	params.removeTexture(index);
	this->fileField[index]->clear();
	update(params, selection);
}

void MaterialEditor::update(ShaderParams params, unsigned index)
{
	this->shared->updateMaterial(params, index);
	this->editor->getPlant()->updateMaterial(params.getMaterial(), index);
	this->editor->change();
	this->materialViewer->updateMaterial(index);
}
