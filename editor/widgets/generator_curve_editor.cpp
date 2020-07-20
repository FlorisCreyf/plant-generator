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
	connect(this->editor, SIGNAL(selectionChanged()),
		this, SLOT(setFields()));
}

void GeneratorCurveEditor::createSelectionBar()
{
	this->selectionBox = new QComboBox(this);
	this->selectionBox->addItem(tr("Stem Density"));
	this->selectionBox->addItem(tr("Leaf Density"));
	this->selectionBox->installEventFilter(this);
	this->selectionBox->view()->installEventFilter(this);
	this->layout->addWidget(this->selectionBox);
	connect(this->selectionBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(select()));

	this->nodeSelectionBox = new QComboBox(this);
	this->nodeSelectionBox->installEventFilter(this);
	this->layout->addWidget(this->nodeSelectionBox);
	connect(this->nodeSelectionBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(select()));
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
	pg::DerivationTree dvnTree = stem->getDerivation();
	std::vector<std::string> names = dvnTree.getNames();
	this->nodeSelectionBox->clear();
	if (names.empty())
		this->nodeSelectionBox->addItem("1");
	else
		for (std::string name : names) {
			QString item = QString::fromStdString(name);
			this->nodeSelectionBox->addItem(item);
		}

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
	pg::DerivationTree dvnTree = stem->getDerivation();
	std::string name = this->nodeSelectionBox->currentText().toStdString();
	pg::DerivationNode *root = dvnTree.get(name);
	if (root) {
		if (index == 0)
			setSpline(root->getData().stemDensityCurve);
		else
			setSpline(root->getData().leafDensityCurve);
	} else {
		Spline spline;
		spline.setDefault(1);
		setSpline(spline);
	}

	this->degree->blockSignals(true);
	this->degree->setCurrentIndex(this->spline.getDegree() == 3 ? 1 : 0);
	this->degree->blockSignals(false);

	enable(true);
}

void GeneratorCurveEditor::enable(bool enable)
{
	this->degree->setEnabled(enable);
	this->nodeSelectionBox->setEnabled(enable);
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
		updateDerivation();
		this->generate->execute();
		this->editor->change();
	}
	this->viewer->change(this->spline, this->selection);
}

void GeneratorCurveEditor::updateDerivation()
{
	std::string name = this->nodeSelectionBox->currentText().toStdString();
	auto instances = this->editor->getSelection()->getStemInstances();
	for (auto instance : instances) {
		int index = this->selectionBox->currentIndex();
		pg::Stem *stem = instance.first;
		pg::DerivationTree dvnTree = stem->getDerivation();
		pg::DerivationNode *dvnNode = dvnTree.get(name);
		if (!dvnNode)
			dvnNode = dvnTree.createRoot();
		pg::Derivation dvn = dvnNode->getData();
		if (index == 0)
			dvn.stemDensityCurve = this->spline;
		else
			dvn.leafDensityCurve = this->spline;
		dvnNode->setData(dvn);
		stem->setDerivation(dvnTree);
	}
}

bool GeneratorCurveEditor::eventFilter(QObject *object, QEvent *event)
{
	bool accepted = CurveEditor::eventFilter(object, event);
	if (event->type() == QEvent::FocusOut) {
		bool focused = isDescendant(QApplication::focusWidget());
		if (!focused && this->generate) {
			this->editor->add(this->generate);
			/* The history will delete the command. */
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
