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

#include "property_editor.h"

PropertyEditor::PropertyEditor(
	SharedResources *shared, Editor *editor, QWidget *parent) :
	QWidget(parent)
{
	this->editor = editor;
	this->stemEditor = new StemEditor(shared, editor, this);
	this->leafEditor = new LeafEditor(shared, editor, this);
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetMinimumSize);
	layout->setSpacing(0);
	layout->setMargin(0);
	layout->addWidget(this->stemEditor);
	layout->addWidget(this->leafEditor);
	layout->addStretch(1);
	connect(this->editor, SIGNAL(selectionChanged()),
		this, SLOT(setFields()));
}

void PropertyEditor::bind(CurveEditor *curveEditor)
{
	this->stemEditor->bind(curveEditor);
}

void PropertyEditor::setFields()
{
	auto leafInstances = this->editor->getSelection()->getLeafInstances();
	auto stemInstances = this->editor->getSelection()->getStemInstances();
	this->stemEditor->setFields(stemInstances);
	this->leafEditor->setFields(leafInstances);
}

void PropertyEditor::addMaterial(ShaderParams params)
{
	bool added = false;
	added = this->stemEditor->addMaterial(params);
	added = this->leafEditor->addMaterial(params);
	if (added)
		this->editor->getPlant()->addMaterial(params.getMaterial());
	editor->change();
}

void PropertyEditor::removeMaterial(unsigned index)
{
	this->stemEditor->removeMaterial(index);
	this->leafEditor->removeMaterial(index);
	this->editor->getPlant()->removeMaterial(index);
	this->editor->change();
}

void PropertyEditor::updateMaterials()
{
	this->stemEditor->updateMaterials();
	this->leafEditor->updateMaterials();
}

void PropertyEditor::addMesh(pg::Geometry geom)
{
	this->leafEditor->addMesh(geom);
	this->editor->change();
}

void PropertyEditor::updateMesh(pg::Geometry geom, unsigned index)
{
	this->leafEditor->updateMesh(geom, index);
}

void PropertyEditor::removeMesh(unsigned index)
{
	this->leafEditor->removeMesh(index);
	this->editor->change();
}

QSize PropertyEditor::sizeHint() const
{
	return QSize(350, 200);
}
