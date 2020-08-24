/* Plant Generator
 * Copyright (C) 2020  Floris Creyf
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

#include "property_curve_editor.h"
#include "definitions.h"
#include "item_delegate.h"

PropertyCurveEditor::PropertyCurveEditor(
	SharedResources *shared, KeyMap *keymap, Editor *editor,
	QWidget *parent) : CurveEditor(keymap, parent), editor(editor)
{
	createSelectionBar();
	createInterface(shared);
}

void PropertyCurveEditor::createSelectionBar()
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

void PropertyCurveEditor::reset()
{
	this->selectionBox->clear();
	this->selection.clear();
	this->history.clear();

	this->selectionBox->blockSignals(true);
	auto curves = this->editor->getPlant()->getCurves();
	for (pg::Curve curve : curves) {
		QString name = QString::fromStdString(curve.getName());
		this->selectionBox->addItem(name);
		emit curveAdded(curve);
	}
	this->selectionBox->blockSignals(false);

	select();
}

void PropertyCurveEditor::add()
{
	pg::Curve curve;
	std::string name;
	QString qname;

	for (int i = 1; true; i++) {
		name = "Curve " + std::to_string(i);
		qname = QString::fromStdString(name);
		if (this->selectionBox->findText(qname) == -1)
			break;
	}

	curve.setName(name);
	pg::Spline spline;
	spline.setDefault(0);
	curve.setSpline(spline);
	add(curve);
}

void PropertyCurveEditor::add(pg::Curve curve)
{
	this->selection.clear();
	this->history.clear();
	pg::Plant *plant = this->editor->getPlant();
	plant->addCurve(curve);

	this->selectionBox->blockSignals(true);
	QString name = QString::fromStdString(curve.getName());
	this->selectionBox->addItem(name);
	this->selectionBox->setCurrentIndex(this->selectionBox->findText(name));
	this->selectionBox->blockSignals(false);

	select();
	emit curveAdded(curve);
}

void PropertyCurveEditor::select()
{
	if (this->selectionBox->count()) {
		pg::Plant *plant = editor->getPlant();
		unsigned index = this->selectionBox->currentIndex();
		pg::Curve curve = plant->getCurve(index);
		setSpline(curve.getSpline());

		this->degree->blockSignals(true);
		int degree = this->spline.getDegree();
		this->degree->setCurrentIndex(degree == 3 ? 1 : 0);
		this->degree->blockSignals(false);
	}
}

void PropertyCurveEditor::rename()
{
	unsigned index = this->selectionBox->currentIndex();
	QString name = this->selectionBox->itemText(index);
	pg::Plant *plant = this->editor->getPlant();
	pg::Curve curve = plant->getCurve(index);
	curve.setName(name.toStdString());
	plant->updateCurve(curve, index);
	emit curveModified(curve, index);
}

void PropertyCurveEditor::remove()
{
	if (this->selectionBox->count() > 1) {
		unsigned index = this->selectionBox->currentIndex();
		QString name = this->selectionBox->currentText();
		pg::Plant *plant = this->editor->getPlant();
		plant->removeCurve(index);
		this->selectionBox->removeItem(index);
		select();
		this->editor->change();
		emit curveRemoved(index);
	}
}

void PropertyCurveEditor::clear()
{
	pg::Plant *plant = this->editor->getPlant();
	int count = this->selectionBox->count();
	while (count > 0) {
		plant->removeCurve(count-1);
		emit curveRemoved(count-1);
		count--;
	}
	this->selectionBox->clear();
	this->selection.clear();
	this->history.clear();
}

void PropertyCurveEditor::change(bool curveChanged)
{
	this->viewer->change(this->spline, this->selection);
	if (curveChanged) {
		pg::Plant *plant = this->editor->getPlant();
		unsigned index = this->selectionBox->currentIndex();
		pg::Curve curve = plant->getCurve(index);
		curve.setSpline(this->spline);
		plant->updateCurve(curve, index);
		this->editor->change();
	}
}
