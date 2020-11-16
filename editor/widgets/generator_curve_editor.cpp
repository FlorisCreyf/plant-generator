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

#include "generator_curve_editor.h"

using pg::Spline;

GeneratorCurveEditor::GeneratorCurveEditor(
	SharedResources *shared, KeyMap *keymap, Editor *editor,
	QWidget *parent) : CurveEditor(keymap, parent), editor(editor),
	generate(nullptr)
{
	createSelectionBar();
	createInterface(shared);
	this->degree->installEventFilter(this);
	this->degree->view()->installEventFilter(this);
	connect(this->editor, &Editor::selectionChanged,
		this, &GeneratorCurveEditor::setFields);
}

void GeneratorCurveEditor::createSelectionBar()
{
	this->nodeSelectionBox = new QComboBox(this);
	this->nodeSelectionBox->installEventFilter(this);
	this->layout->addWidget(this->nodeSelectionBox);
	connect(this->nodeSelectionBox,
		QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &GeneratorCurveEditor::select);

	this->selectionBox = new QComboBox(this);
	this->selectionBox->addItem("Stem Density");
	this->selectionBox->addItem("Leaf Density");
	this->selectionBox->installEventFilter(this);
	this->selectionBox->view()->installEventFilter(this);
	this->layout->addWidget(this->selectionBox);
	connect(this->selectionBox,
		QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &GeneratorCurveEditor::select);
}

void GeneratorCurveEditor::setFields()
{
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty()) {
		clear();
		enable(false);
		return;
	}

	pg::Stem *stem = instances.begin()->first;
	pg::ParameterTree tree = stem->getParameterTree();
	std::vector<std::string> names = tree.getNames();
	this->nodeSelectionBox->clear();
	this->nodeSelectionBox->addItem("");
	for (std::string name : names) {
		QString item = QString::fromStdString(name);
		this->nodeSelectionBox->addItem(item);
	}
	if (!names.empty())
		this->nodeSelectionBox->setCurrentIndex(1);

	select();
}

void GeneratorCurveEditor::select()
{
	clear();
	auto instances = this->editor->getSelection()->getStemInstances();
	if (instances.empty())
		return;

	int index = this->selectionBox->currentIndex();
	pg::Stem *stem = instances.begin()->first;
	pg::ParameterTree tree = stem->getParameterTree();

	if (this->nodeSelectionBox->currentIndex() > 0) {
		std::string name;
		name = this->nodeSelectionBox->currentText().toStdString();
		pg::ParameterNode *node = tree.get(name);
		if (index == 0)
			setSpline(node->getData().densityCurve);
		else
			setSpline(node->getData().leaf.densityCurve);
	} else
		this->viewer->clear();

	this->degree->blockSignals(true);
	this->degree->setCurrentIndex(this->spline.getDegree() == 3 ? 1 : 0);
	this->degree->blockSignals(false);
	enable(true);
}

void GeneratorCurveEditor::enable(bool enable)
{
	this->degree->setEnabled(enable);
	this->nodeSelectionBox->setEnabled(enable);
	this->selectionBox->setEnabled(enable);
}

void GeneratorCurveEditor::clear()
{
	this->selection.clear();
	this->history.clear();
	this->viewer->clear();
}

void GeneratorCurveEditor::change(bool curveChanged)
{
	Selection *selection = this->editor->getSelection();
	auto instances = selection->getStemInstances();
	if (curveChanged && !instances.empty()) {
		if (!this->generate)
			this->generate = new Generate(selection);
		updateParameterTree();
		this->generate->execute();
		this->editor->change();
	}
	this->viewer->change(this->spline, this->selection);
}

void GeneratorCurveEditor::updateParameterTree()
{
	std::string name = this->nodeSelectionBox->currentText().toStdString();
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto instance : instances) {
		int index = this->selectionBox->currentIndex();
		pg::Stem *stem = instance.first;
		pg::ParameterTree tree = stem->getParameterTree();

		if (!tree.getRoot())
			continue;

		if (this->nodeSelectionBox->currentIndex() > 0) {
			pg::ParameterNode *node = tree.get(name);
			if (!node)
				continue;
			pg::StemData data = node->getData();
			if (index == 0)
				data.densityCurve = this->spline;
			else
				data.leaf.densityCurve = this->spline;
			node->setData(data);
			stem->setParameterTree(tree);
		}
	}
}

bool GeneratorCurveEditor::eventFilter(QObject *object, QEvent *event)
{
	bool accepted = CurveEditor::eventFilter(object, event);
	if (event->type() == QEvent::FocusOut) {
		bool focused = isDescendant(QApplication::focusWidget());
		if (!focused && this->generate) {
			this->editor->add(this->generate);
			this->generate = nullptr;
		}
	}
	return accepted;
}

bool GeneratorCurveEditor::isDescendant(QWidget *widget)
{
	return isAncestorOf(widget) ||
		widget == this->degree->view() ||
		widget == this->selectionBox->view() ||
		widget == this->nodeSelectionBox->view();
}
