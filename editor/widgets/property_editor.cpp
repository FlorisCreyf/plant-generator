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
#include "definitions.h"

PropertyEditor::PropertyEditor(
	SharedResources *shared, Editor *editor, QWidget *parent) :
	QWidget(parent),
	editor(editor),
	stemEditor(new StemEditor(shared, editor, this)),
	leafEditor(new LeafEditor(shared, editor, this))
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetMinimumSize);
	layout->setSpacing(0);
	layout->setMargin(0);
	layout->addWidget(this->stemEditor);
	layout->addWidget(this->leafEditor);
	layout->addStretch(1);
	connect(this->editor, &Editor::selectionChanged,
		this, &PropertyEditor::setFields);
}

void PropertyEditor::setFields()
{
	auto leafInstances = this->editor->getSelection()->getLeafInstances();
	auto stemInstances = this->editor->getSelection()->getStemInstances();
	this->stemEditor->setFields(stemInstances);
	this->leafEditor->setFields(leafInstances);
}

void PropertyEditor::clearOptions()
{
	this->stemEditor->clearOptions();
	this->leafEditor->clearOptions();
}

void PropertyEditor::addCurve(pg::Curve curve)
{
	this->stemEditor->addCurve(curve);
}

void PropertyEditor::updateCurve(pg::Curve curve, unsigned index)
{
	this->stemEditor->updateCurve(curve, index);
}

void PropertyEditor::removeCurve(unsigned index)
{
	this->stemEditor->removeCurve(index);
}

void PropertyEditor::addMaterial(ShaderParams params)
{
	this->stemEditor->addMaterial(params);
	this->leafEditor->addMaterial(params);
}

void PropertyEditor::removeMaterial(unsigned index)
{
	this->stemEditor->removeMaterial(index);
	this->leafEditor->removeMaterial(index);
}

void PropertyEditor::updateMaterials()
{
	this->stemEditor->updateMaterials();
	this->leafEditor->updateMaterials();
}

void PropertyEditor::addMesh(pg::Geometry geom)
{
	this->leafEditor->addMesh(geom);
}

void PropertyEditor::updateMesh(pg::Geometry geom, unsigned index)
{
	this->leafEditor->updateMesh(geom, index);
}

void PropertyEditor::removeMesh(unsigned index)
{
	this->leafEditor->removeMesh(index);
}

QSize PropertyEditor::sizeHint() const
{
	return QSize(UI_WIDGET_WIDTH, UI_WIDGET_HEIGHT);
}
