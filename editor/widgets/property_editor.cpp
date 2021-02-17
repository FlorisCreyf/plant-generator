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

#include "property_editor.h"
#include "form.h"

using pg::Leaf;
using pg::Plant;
using pg::PatternGenerator;
using pg::Stem;
using pg::Vec2;
using pg::Vec3;
using std::map;
using std::set;
using std::string;
using std::vector;

PropertyEditor::PropertyEditor(
	SharedResources *shared, Editor *editor, QWidget *parent) :
	QWidget(parent),
	editor(editor),
	shared(shared),
	saveStem(nullptr),
	sameAsCurrent(false)
{
	createInterface();
	connect(this->editor, &Editor::selectionChanged,
		this, &PropertyEditor::setFields);
}

QSize PropertyEditor::sizeHint() const
{
	return QSize(UI_WIDGET_WIDTH, UI_WIDGET_HEIGHT);
}

void PropertyEditor::createInterface()
{
	this->dl[Radius] = new QLabel("Radius");
	this->dl[MinRadius] = new QLabel("Min Radius");
	this->dl[CollarX] = new QLabel("Collar.X");
	this->dl[CollarY] = new QLabel("Collar.Y");
	this->dl[ScaleX] = new QLabel("Scale.X");
	this->dl[ScaleY] = new QLabel("Scale.Y");
	this->dl[ScaleZ] = new QLabel("Scale.Z");

	for (int i = 0; i < DSize; i++) {
		this->dv[i] = new DoubleSpinBox(this);
		this->dv[i]->setSingleStep(0.001);
		this->dv[i]->setDecimals(3);
		connect(this->dv[i], &QDoubleSpinBox::editingFinished,
			this, &PropertyEditor::finishChanging);
	}

	connect(this->dv[Radius],
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		this, [&] (double v) {changeStem([] (Stem *s, double v) {
			s->setMaxRadius(v);
		}, v, this->dl[Radius]);});

	connect(this->dv[MinRadius],
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		this, [&] (double v) {changeStem([] (Stem *s, double v) {
			s->setMinRadius(v);
		}, v, this->dl[MinRadius]);});

	connect(this->dv[CollarX],
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		this, [&] (double v) {changeStem([] (Stem *s, double v) {
			s->setSwelling(Vec2(v, s->getSwelling().y));
		}, v, this->dl[CollarX]);});

	connect(this->dv[CollarY],
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		this, [&] (double v) {changeStem([] (Stem *s, double v) {
			s->setSwelling(Vec2(s->getSwelling().x, v));
		}, v, this->dl[CollarY]);});

	connect(this->dv[ScaleX],
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		this, [&] (double v) {changeLeaf([] (Leaf *l, double v) {
			Vec3 s = l->getScale();
			l->setScale(Vec3(v, s.y, s.z));
		}, v, this->dl[ScaleX]);});

	connect(this->dv[ScaleY],
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		this, [&] (double v) {changeLeaf([] (Leaf *l, double v) {
			Vec3 s = l->getScale();
			l->setScale(Vec3(s.x, v, s.z));
		}, v, this->dl[ScaleY]);});

	connect(this->dv[ScaleZ],
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		this, [&] (double v) {changeLeaf([] (Leaf *l, double v) {
			Vec3 s = l->getScale();
			l->setScale(Vec3(s.x, s.y, v));
		}, v, this->dl[ScaleZ]);});

	this->il[SectionDivisions] = new QLabel("Section Divisions");
	this->il[PathDivisions] = new QLabel("Path Divisions");
	this->il[CollarDivisions] = new QLabel("Collar Divisions");
	for (int i = 0; i < ISize; i++) {
		this->iv[i] = new SpinBox(this);
		connect(this->iv[i], &QSpinBox::editingFinished,
			this, &PropertyEditor::finishChanging);
	}

	connect(this->iv[PathDivisions],
		QOverload<int>::of(&QSpinBox::valueChanged),
		this, [&] (int v) {changeStem([] (Stem *s, int v) {
			pg::Path path = s->getPath();
			path.setDivisions(v);
			s->setPath(path);
		}, v, this->il[PathDivisions]);});

	connect(this->iv[SectionDivisions],
		QOverload<int>::of(&QSpinBox::valueChanged),
		this, [&] (int v) {changeStem([] (Stem *s, int v) {
			s->setSectionDivisions(v);
		}, v, this->il[SectionDivisions]);});

	connect(this->iv[CollarDivisions],
		QOverload<int>::of(&QSpinBox::valueChanged),
		this, [&] (int v) {changeStem([] (Stem *s, int v) {
			s->setCollarDivisions(v);
		}, v, this->il[CollarDivisions]);});

	this->cl[RadiusCurve] = new QLabel("Radius Curve");
	this->cl[Degree] = new QLabel("Degree");
	this->cl[StemMaterial] = new QLabel("Material");
	this->cl[CapMaterial] = new QLabel("Cap Material");
	this->cl[LeafMaterial] = new QLabel("Leaf Material");
	this->cl[Mesh] = new QLabel("Mesh");
	for (int i = 0; i < CSize; i++)
		this->cv[i] = new ComboBox(this);

	connect(this->cv[RadiusCurve],
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, [&] (int v) {
			changeStem([] (Stem *s, int v) {
				s->setRadiusCurve(v);
			}, v, this->cl[RadiusCurve]);
			finishChanging();
		});

	connect(this->cv[Degree],
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, [&] (int v) {
			this->editor->getSelection()->clearPoints();
			changeStem([&] (Stem *s, int v) {
				int degree = v == 1 ? 3 : 1;
				pg::Path path = s->getPath();
				pg::Spline spline = path.getSpline();
				spline.adjust(degree);
				path.setSpline(spline);
				s->setPath(path);
			}, v, this->cl[Degree]);
			finishChanging();
		});

	connect(this->cv[StemMaterial],
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, [&] (int v) {
			changeStem([] (Stem *s, int v) {
				s->setMaterial(Stem::Outer, v);
			}, v, this->cl[StemMaterial]);
			finishChanging();
		});

	connect(this->cv[CapMaterial],
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, [&] (int v) {
			changeStem([] (Stem *s, int v) {
				s->setMaterial(Stem::Inner, v);
			}, v, this->cl[CapMaterial]);
			finishChanging();
		});

	connect(this->cv[LeafMaterial],
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, [&] (int v) {
			changeLeaf([] (Leaf *l, int v) {
				l->setMaterial(v);
			}, v, this->cl[LeafMaterial]);
			finishChanging();
		});

	connect(this->cv[Mesh],
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, [&] (int v) {
			changeLeaf([] (Leaf *l, int v) {
				l->setMesh(v);
			}, v, this->cl[Mesh]);
			finishChanging();
		});

	this->bl[CustomStem] = new QLabel("Manual");
	this->bl[CustomLeaf] = new QLabel("Manual");
	for (int i = 0; i < BSize; i++)
		this->bv[i] = new QCheckBox(this);

	connect(this->bv[CustomStem], &QCheckBox::stateChanged,
		this, [&] (int v) {
			changeStem([] (Stem *s, int v) {
				s->setCustom(v);
			}, v, this->bl[CustomStem]);
			finishChanging();
		});

	connect(this->bv[CustomLeaf], &QCheckBox::stateChanged,
		this, [&] (int v) {
			changeLeaf([] (Leaf *l, int v) {
				l->setCustom(v);
			}, v, this->bl[CustomLeaf]);
			finishChanging();
		});

	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setMargin(0);
	layout->setSpacing(0);
	createStemInterface(layout);
	createLeafInterface(layout);
	layout->addStretch(1);
}

inline QGroupBox *createGroup(const char *name)
{
	QGroupBox *group = new QGroupBox(name);
	group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	return group;
}

inline QFormLayout *createForm(QGroupBox *box)
{
	QFormLayout *form = new QFormLayout(box);
	form->setSpacing(UI_FORM_SPACING);
	form->setMargin(UI_FORM_MARGIN);
	return form;
}

void PropertyEditor::createStemInterface(QBoxLayout *layout)
{
	this->stemGroup = createGroup("Stem");
	layout->addWidget(this->stemGroup);
	QFormLayout *form = createForm(this->stemGroup);
	form->addRow(this->dl[Radius], this->dv[Radius]);
	this->dv[MinRadius]->setSingleStep(0.001);
	form->addRow(this->dl[MinRadius], this->dv[MinRadius]);
	form->addRow(this->cl[RadiusCurve], this->cv[RadiusCurve]);
	this->iv[PathDivisions]->setMinimum(0);
	form->addRow(this->il[PathDivisions], this->iv[PathDivisions]);
	this->iv[SectionDivisions]->setMinimum(3);
	form->addRow(this->il[SectionDivisions], this->iv[SectionDivisions]);
	this->iv[CollarDivisions]->setMinimum(0);
	form->addRow(this->il[CollarDivisions], this->iv[CollarDivisions]);
	this->cv[Degree]->addItem(QString("Linear"));
	this->cv[Degree]->addItem(QString("Cubic"));
	form->addRow(this->cl[Degree], this->cv[Degree]);
	form->addRow(this->cl[StemMaterial], this->cv[StemMaterial]);
	form->addRow(this->cl[CapMaterial], this->cv[CapMaterial]);
	this->dv[CollarX]->setSingleStep(0.1);
	form->addRow(this->dl[CollarX], this->dv[CollarX]);
	this->dv[CollarY]->setSingleStep(0.1);
	form->addRow(this->dl[CollarY], this->dv[CollarY]);
	form->addRow(this->bl[CustomStem], this->bv[CustomStem]);
	setFormLayout(form);
}

void PropertyEditor::createLeafInterface(QBoxLayout *layout)
{
	this->leafGroup = createGroup("Leaf");
	layout->addWidget(this->leafGroup);
	QFormLayout *form = createForm(this->leafGroup);
	this->dv[ScaleX]->setSingleStep(0.1);
	form->addRow(this->dl[ScaleX], this->dv[ScaleX]);
	this->dv[ScaleY]->setSingleStep(0.1);
	form->addRow(this->dl[ScaleY], this->dv[ScaleY]);
	this->dv[ScaleZ]->setSingleStep(0.1);
	form->addRow(this->dl[ScaleZ], this->dv[ScaleZ]);
	form->addRow(this->cl[LeafMaterial], this->cv[LeafMaterial]);
	form->addRow(this->cl[Mesh], this->cv[Mesh]);
	form->addRow(this->bl[CustomLeaf], this->bv[CustomLeaf]);
	enableLeafFields(false);
	setFormLayout(form);
}

void PropertyEditor::blockSignals(bool block)
{
	for (int i = 0; i < DSize; i++)
		this->dv[i]->blockSignals(block);
	for (int i = 0; i < ISize; i++)
		this->iv[i]->blockSignals(block);
	for (int i = 0; i < CSize; i++)
		this->cv[i]->blockSignals(block);
	for (int i = 0; i < BSize; i++)
		this->bv[i]->blockSignals(block);
}

void PropertyEditor::enableStemFields(bool enable)
{
	if (!enable) {
		indicateSimilarities(this->dl[Radius]);
		indicateSimilarities(this->dl[MinRadius]);
		indicateSimilarities(this->cl[RadiusCurve]);
		indicateSimilarities(this->il[SectionDivisions]);
		indicateSimilarities(this->il[CollarDivisions]);
		indicateSimilarities(this->il[PathDivisions]);
		indicateSimilarities(this->cl[Degree]);
		indicateSimilarities(this->cl[StemMaterial]);
		indicateSimilarities(this->cl[CapMaterial]);
		indicateSimilarities(this->dl[CollarX]);
		indicateSimilarities(this->dl[CollarY]);
		indicateSimilarities(this->bl[CustomStem]);
	}
	this->dv[Radius]->setEnabled(enable);
	this->dv[MinRadius]->setEnabled(enable);
	this->cv[RadiusCurve]->setEnabled(enable);
	this->iv[SectionDivisions]->setEnabled(enable);
	this->iv[CollarDivisions]->setEnabled(enable);
	this->iv[PathDivisions]->setEnabled(enable);
	this->cv[Degree]->setEnabled(enable);
	this->cv[StemMaterial]->setEnabled(enable);
	this->cv[CapMaterial]->setEnabled(enable);
	this->dv[CollarX]->setEnabled(enable);
	this->dv[CollarY]->setEnabled(enable);
	this->bv[CustomStem]->setEnabled(enable);
}

void PropertyEditor::enableLeafFields(bool enable)
{
	if (!enable) {
		indicateSimilarities(this->dl[ScaleX]);
		indicateSimilarities(this->dl[ScaleY]);
		indicateSimilarities(this->dl[ScaleZ]);
		indicateSimilarities(this->cl[LeafMaterial]);
		indicateSimilarities(this->cl[Mesh]);
		indicateSimilarities(this->bl[CustomLeaf]);
	}
	this->dv[ScaleX]->setEnabled(enable);
	this->dv[ScaleY]->setEnabled(enable);
	this->dv[ScaleZ]->setEnabled(enable);
	this->cv[LeafMaterial]->setEnabled(enable);
	this->cv[Mesh]->setEnabled(enable);
	this->bv[CustomLeaf]->setEnabled(enable);
}

void PropertyEditor::beginChanging(QLabel *label)
{
	indicateSimilarities(label);
	if (!this->saveStem) {
		Selection *selection = this->editor->getSelection();
		this->saveStem = new SaveStem(selection);
		this->saveStem->execute();
	}
	this->sameAsCurrent = this->saveStem->isSameAsCurrent();
}

void PropertyEditor::finishChanging()
{
	if (this->saveStem && !this->sameAsCurrent)
		this->editor->getHistory()->add(this->saveStem);
	else if (this->saveStem)
		delete this->saveStem;
	this->saveStem = nullptr;
	this->sameAsCurrent = false;
}

void PropertyEditor::setFields()
{
	if (this->saveStem)
		finishChanging();
	auto leafInstances = this->editor->getSelection()->getLeafInstances();
	auto stemInstances = this->editor->getSelection()->getStemInstances();
	blockSignals(true);
	setStemFields(stemInstances);
	setLeafFields(leafInstances);
	blockSignals(false);
}

void PropertyEditor::setStemFields(const map<Stem *, PointSelection> &instances)
{
	if (instances.empty()) {
		enableStemFields(false);
		return;
	}

	Stem *stem = instances.begin()->first;
	for (auto it = ++instances.begin(); it != instances.end(); it++) {
		Stem *s = it->first;

		if (s->isCustom() != stem->isCustom())
			indicateDifferences(this->bl[CustomStem]);
		if (s->getSectionDivisions() != stem->getSectionDivisions())
			indicateDifferences(this->il[SectionDivisions]);
		if (s->getCollarDivisions() != stem->getCollarDivisions())
			indicateDifferences(this->il[CollarDivisions]);
		if (s->getMaxRadius() != stem->getMaxRadius())
			indicateDifferences(this->dl[Radius]);
		if (s->getMinRadius() != stem->getMinRadius())
			indicateDifferences(this->dl[MinRadius]);
		if (s->getRadiusCurve() != stem->getRadiusCurve())
			indicateDifferences(this->cl[RadiusCurve]);

		const pg::Path &path1 = s->getPath();
		const pg::Path &path2 = stem->getPath();
		if (path1.getDivisions() != path2.getDivisions())
			indicateDifferences(this->il[PathDivisions]);

		const pg::Spline &spline1 = path1.getSpline();
		const pg::Spline &spline2 = path2.getSpline();
		if (spline1.getDegree() != spline2.getDegree())
			indicateDifferences(this->cl[Degree]);

		unsigned material1 = s->getMaterial(Stem::Outer);
		unsigned material2 = stem->getMaterial(Stem::Outer);
		if (material1 != material2)
			indicateDifferences(this->cl[StemMaterial]);
		material1 = s->getMaterial(Stem::Inner);
		material2 = stem->getMaterial(Stem::Inner);
		if (material1 != material2)
			indicateDifferences(this->cl[CapMaterial]);

		if (s->getSwelling().x != stem->getSwelling().x)
			indicateDifferences(this->dl[CollarX]);
		if (s->getSwelling().y != stem->getSwelling().y)
			indicateDifferences(this->dl[CollarY]);
	}

	setStemValues(stem);
}

void PropertyEditor::setStemValues(const Stem *stem)
{
	enableStemFields(true);
	this->iv[SectionDivisions]->setValue(stem->getSectionDivisions());
	this->iv[CollarDivisions]->setValue(stem->getCollarDivisions());
	this->iv[PathDivisions]->setValue(stem->getPath().getDivisions());
	this->dv[Radius]->setValue(stem->getMaxRadius());
	this->dv[MinRadius]->setValue(stem->getMinRadius());
	this->cv[RadiusCurve]->setCurrentIndex(stem->getRadiusCurve());
	this->dv[CollarX]->setValue(stem->getSwelling().x);
	this->dv[CollarY]->setValue(stem->getSwelling().y);
	this->bv[CustomStem]->setCheckState(
		stem->isCustom() ? Qt::Checked : Qt::Unchecked);

	if (stem->getPath().getSpline().getDegree() == 3)
		this->cv[Degree]->setCurrentIndex(1);
	else
		this->cv[Degree]->setCurrentIndex(0);

	unsigned index = stem->getMaterial(Stem::Outer);
	string name = this->shared->getMaterial(index).getName();
	this->cv[StemMaterial]->setCurrentText(name.c_str());

	index = stem->getMaterial(Stem::Inner);
	name = shared->getMaterial(index).getName();
	this->cv[CapMaterial]->setCurrentText(name.c_str());
}

void PropertyEditor::setLeafFields(const map<Stem *, set<size_t>> &instances)
{
	if (instances.empty()) {
		enableLeafFields(false);
		return;
	}

	Stem *stem = instances.rbegin()->first;
	Leaf *leaf = stem->getLeaf(*instances.rbegin()->second.begin());
	for (auto it = ++instances.begin(); it != instances.end(); it++) {
		Stem *stem = it->first;
		for (size_t index : it->second) {
			Leaf *l = stem->getLeaf(index);
			if (l->isCustom() != leaf->isCustom())
				indicateDifferences(this->il[CustomLeaf]);
			if (l->getScale().x != leaf->getScale().x)
				indicateDifferences(this->dl[ScaleX]);
			if (l->getScale().y != leaf->getScale().y)
				indicateDifferences(this->dl[ScaleY]);
			if (l->getScale().z != leaf->getScale().z)
				indicateDifferences(this->dl[ScaleZ]);
			if (l->getMaterial() != leaf->getMaterial())
				indicateDifferences(this->cl[LeafMaterial]);
			if (l->getMesh() != leaf->getMesh())
				indicateDifferences(this->cl[Mesh]);
		}
	}

	setLeafValues(leaf);
}

void PropertyEditor::setLeafValues(const Leaf *leaf)
{
	enableLeafFields(true);
	this->dv[ScaleX]->setValue(leaf->getScale().x);
	this->dv[ScaleY]->setValue(leaf->getScale().y);
	this->dv[ScaleZ]->setValue(leaf->getScale().z);
	this->bv[CustomLeaf]->setCheckState(
		leaf->isCustom() ? Qt::Checked : Qt::Unchecked);

	string name = this->shared->getMaterial(leaf->getMaterial()).getName();
	this->cv[LeafMaterial]->setCurrentText(name.c_str());

	Plant *plant = this->editor->getPlant();
	name = plant->getLeafMesh(leaf->getMesh()).getName();
	this->cv[Mesh]->setCurrentText(name.c_str());
}

void PropertyEditor::clearOptions()
{
	blockSignals(true);
	this->cv[StemMaterial]->clear();
	this->cv[CapMaterial]->clear();
	this->cv[RadiusCurve]->clear();
	this->cv[LeafMaterial]->clear();
	this->cv[Mesh]->clear();
	blockSignals(false);
}

void PropertyEditor::addMaterial(ShaderParams params)
{
	QString name = QString::fromStdString(params.getName());
	if (this->cv[StemMaterial]->findText(name) < 0) {
		blockSignals(true);
		this->cv[StemMaterial]->addItem(name);
		this->cv[CapMaterial]->addItem(name);
		this->cv[LeafMaterial]->addItem(name);
		blockSignals(false);
	}
}

void PropertyEditor::updateMaterials()
{
	unsigned size = this->shared->getMaterialCount();
	for (unsigned i = 0; i < size; i++) {
		ShaderParams params = this->shared->getMaterial(i);
		QString name = QString::fromStdString(params.getName());
		this->cv[StemMaterial]->setItemText(i, name);
		this->cv[CapMaterial]->setItemText(i, name);
		this->cv[LeafMaterial]->setItemText(i, name);
	}
}

void PropertyEditor::removeMaterial(int index)
{
	blockSignals(true);
	int stemIndex = this->cv[StemMaterial]->currentIndex();
	int capIndex = this->cv[CapMaterial]->currentIndex();
	int leafIndex = this->cv[LeafMaterial]->currentIndex();
	this->cv[StemMaterial]->setCurrentIndex((index != stemIndex) * index);
	this->cv[CapMaterial]->setCurrentIndex((index != capIndex) * index);
	this->cv[LeafMaterial]->setCurrentIndex((index != leafIndex) * index);
	this->cv[StemMaterial]->removeItem(index);
	this->cv[CapMaterial]->removeItem(index);
	this->cv[LeafMaterial]->removeItem(index);
	blockSignals(false);
}

void PropertyEditor::addCurve(pg::Curve curve)
{
	QString name = QString::fromStdString(curve.getName());
	if (this->cv[RadiusCurve]->findText(name) < 0) {
		this->cv[RadiusCurve]->blockSignals(true);
		this->cv[RadiusCurve]->addItem(name);
		this->cv[RadiusCurve]->blockSignals(false);
	}
}

void PropertyEditor::updateCurve(pg::Curve curve, int index)
{
	QString name = QString::fromStdString(curve.getName());
	this->cv[RadiusCurve]->setItemText(index, name);
}

void PropertyEditor::removeCurve(int index)
{
	int curveIndex = this->cv[RadiusCurve]->currentIndex();
	this->cv[RadiusCurve]->blockSignals(true);
	this->cv[RadiusCurve]->setCurrentIndex((index != curveIndex) * index);
	this->cv[RadiusCurve]->removeItem(index);
	this->cv[RadiusCurve]->blockSignals(false);
}

void PropertyEditor::addMesh(pg::Geometry geom)
{
	QString name = QString::fromStdString(geom.getName());
	if (this->cv[Mesh]->findText(name) < 0)
		this->cv[Mesh]->addItem(name);
}

void PropertyEditor::updateMesh(pg::Geometry geom, int index)
{
	QString name = QString::fromStdString(geom.getName());
	this->cv[Mesh]->setItemText(index, name);
}

void PropertyEditor::removeMesh(int index)
{
	int currentIndex = this->cv[Mesh]->currentIndex();
	this->cv[Mesh]->blockSignals(true);
	this->cv[Mesh]->setCurrentIndex((index != currentIndex) * index);
	this->cv[Mesh]->removeItem(index);
	this->cv[Mesh]->blockSignals(false);
}
