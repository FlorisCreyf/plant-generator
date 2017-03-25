/* TreeMaker: 3D tree model editor
 * Copyright (C) 2016-2017  Floris Creyf
 *
 * TreeMaker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TreeMaker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "property_box.h"

PropertyBox::PropertyBox(SharedResources *shared, QWidget *parent) :
		QWidget(parent)
{
	this->shared = shared;
	curveEditor = nullptr;
	activeCurve = nullptr;

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetMinimumSize);
	layout->setSpacing(0);
	layout->setMargin(0);
	createLocalBox(layout);
	layout->addStretch(1);
}

void PropertyBox::createGlobalBox(QVBoxLayout *layout)
{
	QGroupBox *group = new QGroupBox(tr("Tree"));
	QVBoxLayout *groupLayout = new QVBoxLayout(group);
	global = new QTableWidget(this);
	global->setRowCount(0);
	global->setColumnCount(2);
	configureTable(global);
	groupLayout->addStretch(1);
	groupLayout->addWidget(global);
	group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	layout->addWidget(group);
}

void PropertyBox::createLocalBox(QVBoxLayout *layout)
{
	localGroup = new QGroupBox(tr("Stem"));
	QVBoxLayout *groupLayout = new QVBoxLayout(localGroup);
	local = new QTableWidget(this);
	
	seed = new QSpinBox;
	seed->setMaximum(std::numeric_limits<int>::max());
	
	resolution = new QSpinBox;
	radius = new QDoubleSpinBox;
	radiusCB = new CurveButton("Radius", shared, this);
	sections = new QSpinBox;
	stems = new QDoubleSpinBox;
	base = new QDoubleSpinBox;
	
	QWidget *mw = new QWidget();
	QVBoxLayout *mwl = new QVBoxLayout(mw);
	mwl->setSpacing(0);
	mwl->setMargin(0);
	modes[0] = new QRadioButton("Automatic");
	modes[1] = new QRadioButton("Assisted");
	modes[2] = new QRadioButton("Manual");
	mwl->addWidget(modes[0]);
	mwl->addWidget(modes[1]);
	mwl->addWidget(modes[2]);
	
	QWidget *dw = new QWidget();
	QVBoxLayout *dwl = new QVBoxLayout(dw);
	dwl->setSpacing(0);
	dwl->setMargin(0);
	distribution[0] = new QRadioButton("Distribute");
	distribution[1] = new QRadioButton("Fixed");
	dwl->addWidget(distribution[0]);
	dwl->addWidget(distribution[1]);
	
	local->setRowCount(8);
	local->setColumnCount(3);
	radius->setSingleStep(0.01);
	sections->setMinimum(2);
	resolution->setMinimum(5);
	local->setCellWidget(0, 0, new QLabel(tr("Mode")));
	local->setCellWidget(0, 1, mw);
	local->setCellWidget(1, 0, new QLabel(tr("Seed")));
	local->setCellWidget(1, 1, seed);
	local->setCellWidget(2, 0, new QLabel(tr("Radius")));
	local->setCellWidget(2, 1, radius);
	local->setCellWidget(2, 2, createCenteredWidget(radiusCB));
	local->setCellWidget(3, 0, new QLabel(tr("H Sections")));
	local->setCellWidget(3, 1, resolution);
	local->setCellWidget(4, 0, new QLabel(tr("V Sections")));
	local->setCellWidget(4, 1, sections);
	local->setCellWidget(5, 0, new QLabel(tr("Stem Density")));
	local->setCellWidget(5, 1, stems);
	local->setCellWidget(6, 0, new QLabel(tr("Distribution")));
	local->setCellWidget(6, 1, dw);
	local->setCellWidget(7, 0, new QLabel(tr("Base Length")));
	local->setCellWidget(7, 1, base);
	
	configureTable(local);
	groupLayout->addStretch(1);
	groupLayout->addWidget(local);
	localGroup->hide();
	localGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	layout->addWidget(localGroup);
}

void PropertyBox::configureTable(QTableWidget *table)
{	
	table->horizontalHeader()->hide();
	table->verticalHeader()->hide();
	table->setShowGrid(false);
	table->setFocusPolicy(Qt::NoFocus);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	table->setSelectionMode(QAbstractItemView::NoSelection);
	
	for (int i = 0; i < table->rowCount(); i++) {
		auto r = table->cellWidget(i, 1);
		auto l = table->cellWidget(i, 0);
		const char *name = r->metaObject()->className();
		
		if (strcmp(name, "QSpinBox") == 0 || 
				strcmp(name, "QDoubleSpinBox") == 0)
			table->setRowHeight(i, 24);
		else
			table->resizeRowToContents(i);
		
		if (l != nullptr)
			l->setFixedHeight(24);
	}

	{
		QHeaderView *header = table->horizontalHeader();
		if (table->columnCount() == 3)
			header->setSectionResizeMode(2, QHeaderView::Fixed);
		header->setSectionResizeMode(1, QHeaderView::Stretch);
		header->setSectionResizeMode(0, QHeaderView::Fixed);
		header->resizeSection(2, 30);
	}
	
	{
		int h = 0;
		for (int i = 0; i < table->rowCount(); i++)
			h += table->rowHeight(i);
		table->setFixedHeight(h);
		table->resizeColumnToContents(0);
	}
}

QWidget *PropertyBox::createCenteredWidget(QWidget *widget)
{
	QWidget *w = new QWidget(this);
	QHBoxLayout *b = new QHBoxLayout(w);
	b->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	b->setMargin(0);
	b->addWidget(widget);
	return w;
}

QSize PropertyBox::sizeHint() const
{
	return QSize(300, 0);
}

void PropertyBox::setCurve(std::vector<treemaker::Vec3> controls, QString name)
{
	if (name == "Radius") {
		radiusCB->setControls(controls);
		changeRadiusCurve(controls);
	}
}

void PropertyBox::toggleCurve(CurveButton *button)
{
	activeCurve = button;
	curveEditor->setCurve(button->getControls(), button->getName());
}

void PropertyBox::fill(treemaker::Tree &tree, int stem)
{
	treemaker::Flags mode;
	initializingFields = true;
	
	if (stem < 0) {
		localGroup->hide();
		curveEditor->setEnabled(false);
		return;
	} else {
		mode = tree.getMode(stem);
		modes[mode]->toggle();
		
		if (mode == treemaker::AUTOMATIC)
			curveEditor->setEnabled(false);
		else
			curveEditor->setEnabled(true);
		
		localGroup->show();
	}

	resolution->setValue(tree.getResolution(stem));
	sections->setValue(tree.getGeneratedPathSize(stem));
	radius->setValue(tree.getRadius(stem));
	stems->setValue(tree.getStemDensity(stem));
	base->setValue(tree.getBaseLength(stem));
	fillCurveButtons(tree, stem);

	if (activeCurve)
		activeCurve->select();
	
	if (mode == treemaker::MANUAL) {
		auto dis = tree.getDistribution(stem);
		distribution[dis]->toggle();
	}
	
	initializingFields = false;
}

void PropertyBox::fillCurveButtons(treemaker::Tree &tree, int stem)
{
	std::vector<treemaker::Vec3> curve(tree.getRadiusCurveSize(stem));
	tree.getRadiusCurve(stem, &curve[0]);
	radiusCB->setControls(curve);
}

void PropertyBox::updateMode()
{
	auto selected = editor->getSelectedStem();
	auto tree = editor->getTree();
	modes[tree->getMode(selected)]->toggle();
}

void PropertyBox::setAutomaticMode(bool toggled)
{
	if (toggled) {
		if (!initializingFields) {
			auto selected = editor->getSelectedStem();
			auto tree = editor->getTree();
			tree->setMode(selected, treemaker::AUTOMATIC);
			editor->change();
		}
		fill(*editor->getTree(), editor->getSelectedStem());
		for (int i = 1; i < local->rowCount(); i++)
			local->hideRow(i);
	} else
		for (int i = 1; i < local->rowCount(); i++)
			local->showRow(i);
}

void PropertyBox::setAssistedMode(bool toggled)
{
	if (toggled) {
		if (!initializingFields) {
			auto selected = editor->getSelectedStem();
			auto tree = editor->getTree();
			tree->setMode(selected, treemaker::ASSISTED);
			editor->change();
		}
		fill(*editor->getTree(), editor->getSelectedStem());
		local->hideRow(6);
	} else
		local->showRow(6);
}

void PropertyBox::setManualMode(bool toggled)
{
	if (toggled) {
		if (!initializingFields) {
			auto selected = editor->getSelectedStem();
			auto tree = editor->getTree();
			tree->setMode(selected, treemaker::MANUAL);
		}
		fill(*editor->getTree(), editor->getSelectedStem());
		local->hideRow(5);
		local->hideRow(7);
		local->hideRow(1);
	} else {
		local->showRow(5);
		local->showRow(7);
		local->showRow(1);
	}
}

void PropertyBox::setDistributeMode(bool toggled)
{
	if (!initializingFields && toggled) {
		auto selected = editor->getSelectedStem();
		auto tree = editor->getTree();
		tree->setDistribution(selected, treemaker::DISTRIBUTED);
	}
}

void PropertyBox::setFixedMode(bool toggled)
{
	if (!initializingFields && toggled) {
		auto selected = editor->getSelectedStem();
		auto tree = editor->getTree();
		tree->setDistribution(selected, treemaker::FIXED);
	}
}

void PropertyBox::changeResolution(int i)
{
	if (!initializingFields) {
		auto tree = editor->getTree();
		tree->setResolution(editor->getSelectedStem(), i);
		editor->change();
	}
}

void PropertyBox::changeSections(int i)
{
	if (!initializingFields) {
		auto tree = editor->getTree();
		tree->setGeneratedPathSize(editor->getSelectedStem(), i);
		editor->change();
	}
}

void PropertyBox::changeRadius(double d)
{
	if (!initializingFields) {
		auto tree = editor->getTree();
		tree->setRadius(editor->getSelectedStem(), d);
		editor->change();
	}
}

void PropertyBox::changeRadiusCurve(std::vector<treemaker::Vec3> c)
{
	if (!initializingFields) {
		auto tree = editor->getTree();
		auto selectedStem = editor->getSelectedStem();
		tree->setRadiusCurve(selectedStem, &c[0], c.size());
		editor->change();
	}
}

void PropertyBox::changeStemDensity(double d)
{
	if (!initializingFields) {
		auto tree = editor->getTree();
		tree->setStemDensity(editor->getSelectedStem(), d);
		editor->change();
	}
}

void PropertyBox::changeBaseLength(double d)
{
	if (!initializingFields) {
		auto tree = editor->getTree();
		tree->setBaseLength(editor->getSelectedStem(), d);
		editor->change();
	}
}

void PropertyBox::bind(Editor *editor, CurveEditor *curveEditor)
{
	this->curveEditor = curveEditor;
	this->editor = editor;

	connect(modes[0], SIGNAL(toggled(bool)), this,
			SLOT(setAutomaticMode(bool)));
	connect(modes[1], SIGNAL(toggled(bool)), this,
			SLOT(setAssistedMode(bool)));
	connect(modes[2], SIGNAL(toggled(bool)), this,
			SLOT(setManualMode(bool)));
	connect(distribution[0], SIGNAL(toggled(bool)), this,
			SLOT(setDistributeMode(bool)));
	connect(distribution[1], SIGNAL(toggled(bool)), this,
			SLOT(setFixedMode(bool)));
	
	connect(editor, SIGNAL(modeChanged()), this, SLOT(updateMode()));
	connect(editor, SIGNAL(selectionChanged(treemaker::Tree &, int)), this,
			SLOT(fill(treemaker::Tree &, int)));
	
	connect(resolution, SIGNAL(valueChanged(int)), this,
			SLOT(changeResolution(int)));
	connect(sections, SIGNAL(valueChanged(int)), this,
			SLOT(changeSections(int)));
	connect(radius, SIGNAL(valueChanged(double)), this,
			SLOT(changeRadius(double)));
	connect(stems, SIGNAL(valueChanged(double)), this,
			SLOT(changeStemDensity(double)));
	connect(base, SIGNAL(valueChanged(double)), this,
			SLOT(changeBaseLength(double)));

	bindCurveEditor();
}

void PropertyBox::bindCurveEditor()
{
	connect(curveEditor, SIGNAL(curveChanged(std::vector<treemaker::Vec3>, QString)), 
			this, SLOT(setCurve(std::vector<treemaker::Vec3>, QString)));
	connect(radiusCB, SIGNAL(selected(CurveButton *)), this, 
			SLOT(toggleCurve(CurveButton *)));
}
