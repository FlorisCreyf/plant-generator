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
#include "plant_generator/file/wavefront.h"

using pg::Curve;
using pg::Material;
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

PropertyEditor::PropertyEditor(SharedResources *shared, KeyMap *keymap,
	Editor *editor, QWidget *parent) :
	QWidget(parent),
	editor(editor),
	keymap(keymap),
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
	createStemLeafFields();
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setMargin(0);
	layout->setSpacing(0);
	createStemInterface(layout);
	createLeafInterface(layout);
	createCurveInterface(layout);
	createMaterialInterface(layout);
	createMeshInterface(layout);
	createWindInterface(layout);
	layout->addStretch(1);
}

void PropertyEditor::createStemLeafFields()
{
	this->dl[Radius] = new QLabel("Radius");
	this->dl[MinRadius] = new QLabel("Min Radius");
	this->dl[CollarX] = new QLabel("Collar.X");
	this->dl[CollarY] = new QLabel("Collar.Y");
	this->dl[ScaleX] = new QLabel("Scale.X");
	this->dl[ScaleY] = new QLabel("Scale.Y");
	this->dl[ScaleZ] = new QLabel("Scale.Z");
	this->il[SectionDivisions] = new QLabel("Section Divisions");
	this->il[PathDivisions] = new QLabel("Path Divisions");
	this->il[CollarDivisions] = new QLabel("Collar Divisions");
	this->cl[RadiusCurve] = new QLabel("Radius Curve");
	this->cl[PathDegree] = new QLabel("Degree");
	this->cl[StemMaterial] = new QLabel("Material");
	this->cl[CapMaterial] = new QLabel("Cap Material");
	this->cl[LeafMaterial] = new QLabel("Material");
	this->cl[LeafMesh] = new QLabel("Mesh");
	this->bl[CustomStem] = new QLabel("Manual");
	this->bl[CustomLeaf] = new QLabel("Manual");

	for (int i = 0; i < DSize; i++) {
		this->dv[i] = new DoubleSpinBox(this);
		this->dv[i]->setSingleStep(0.001);
		this->dv[i]->setDecimals(3);
		connect(this->dv[i], &QDoubleSpinBox::editingFinished,
			this, &PropertyEditor::finishChanging);
	}
	for (int i = 0; i < ISize; i++) {
		this->iv[i] = new SpinBox(this);
		connect(this->iv[i], &QSpinBox::editingFinished,
			this, &PropertyEditor::finishChanging);
	}
	for (int i = 0; i < CSize; i++)
		this->cv[i] = new ComboBox(this);
	for (int i = 0; i < BSize; i++)
		this->bv[i] = new QCheckBox(this);

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

	connect(this->cv[RadiusCurve],
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, [&] (int v) {
			changeStem([] (Stem *s, int v) {
				s->setRadiusCurve(v);
			}, v, this->cl[RadiusCurve]);
			this->sameAsCurrent = this->saveStem->isSameAsCurrent();
			finishChanging();
		});

	connect(this->cv[PathDegree],
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, [&] (int v) {
			this->editor->getSelection()->clearPoints();
			changeStem([] (Stem *s, int v) {
				int degree = v == 1 ? 3 : 1;
				pg::Path path = s->getPath();
				pg::Spline spline = path.getSpline();
				spline.adjust(degree);
				path.setSpline(spline);
				s->setPath(path);
			}, v, this->cl[PathDegree]);
			this->sameAsCurrent = this->saveStem->isSameAsCurrent();
			finishChanging();
		});

	connect(this->cv[StemMaterial],
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, [&] (int v) {
			changeStem([] (Stem *s, int v) {
				s->setMaterial(Stem::Outer, v);
			}, v, this->cl[StemMaterial]);
			this->sameAsCurrent = this->saveStem->isSameAsCurrent();
			finishChanging();
		});

	connect(this->cv[CapMaterial],
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, [&] (int v) {
			changeStem([] (Stem *s, int v) {
				s->setMaterial(Stem::Inner, v);
			}, v, this->cl[CapMaterial]);
			this->sameAsCurrent = this->saveStem->isSameAsCurrent();
			finishChanging();
		});

	connect(this->cv[LeafMaterial],
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, [&] (int v) {
			changeLeaf([] (Leaf *l, int v) {
				l->setMaterial(v);
			}, v, this->cl[LeafMaterial]);
			this->sameAsCurrent = this->saveStem->isSameAsCurrent();
			finishChanging();
		});

	connect(this->cv[LeafMesh],
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, [&] (int v) {
			changeLeaf([] (Leaf *l, int v) {
				l->setMesh(v);
			}, v, this->cl[LeafMesh]);
			this->sameAsCurrent = this->saveStem->isSameAsCurrent();
			finishChanging();
		});

	connect(this->bv[CustomStem], &QCheckBox::stateChanged,
		this, [&] (int v) {
			changeStem([] (Stem *s, int v) {
				s->setCustom(v);
			}, v, this->bl[CustomStem]);
			this->sameAsCurrent = false;
			finishChanging();
		});

	connect(this->bv[CustomLeaf], &QCheckBox::stateChanged,
		this, [&] (int v) {
			changeLeaf([] (Leaf *l, int v) {
				l->setCustom(v);
			}, v, this->bl[CustomLeaf]);
			this->sameAsCurrent = false;
			finishChanging();
		});
}

void PropertyEditor::createStemInterface(QBoxLayout *layout)
{
	QGroupBox *stemGroup = createGroup("Stem");
	layout->addWidget(stemGroup);
	QFormLayout *form = createForm(stemGroup);
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
	this->cv[PathDegree]->addItem(QString("Linear"));
	this->cv[PathDegree]->addItem(QString("Cubic"));
	form->addRow(this->cl[PathDegree], this->cv[PathDegree]);
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
	QGroupBox *leafGroup = createGroup("Leaf");
	layout->addWidget(leafGroup);
	QFormLayout *form = createForm(leafGroup);
	this->dv[ScaleX]->setSingleStep(0.1);
	form->addRow(this->dl[ScaleX], this->dv[ScaleX]);
	this->dv[ScaleY]->setSingleStep(0.1);
	form->addRow(this->dl[ScaleY], this->dv[ScaleY]);
	this->dv[ScaleZ]->setSingleStep(0.1);
	form->addRow(this->dl[ScaleZ], this->dv[ScaleZ]);
	form->addRow(this->cl[LeafMaterial], this->cv[LeafMaterial]);
	form->addRow(this->cl[LeafMesh], this->cv[LeafMesh]);
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
		indicateSimilarities(this->cl[PathDegree]);
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
	this->cv[PathDegree]->setEnabled(enable);
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
		indicateSimilarities(this->cl[LeafMesh]);
		indicateSimilarities(this->bl[CustomLeaf]);
	}
	this->dv[ScaleX]->setEnabled(enable);
	this->dv[ScaleY]->setEnabled(enable);
	this->dv[ScaleZ]->setEnabled(enable);
	this->cv[LeafMaterial]->setEnabled(enable);
	this->cv[LeafMesh]->setEnabled(enable);
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
	setCurves();
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
			indicateDifferences(this->cl[PathDegree]);

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
		this->cv[PathDegree]->setCurrentIndex(1);
	else
		this->cv[PathDegree]->setCurrentIndex(0);

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
				indicateDifferences(this->cl[LeafMesh]);
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
	this->cv[LeafMesh]->setCurrentText(name.c_str());
}

void PropertyEditor::createCurveInterface(QBoxLayout *layout)
{
	QGroupBox *curveGroup = createGroup("Curve");
	layout->addWidget(curveGroup);
	QBoxLayout *vlayout = new QVBoxLayout(curveGroup);
	vlayout->setMargin(0);
	vlayout->setSpacing(0);
	this->curveEditor = new CurveEditor(this->shared, this->keymap, this);
	vlayout->addWidget(this->curveEditor);
	QFormLayout *form = createForm(vlayout);
	this->curveDegree = new ComboBox();
	this->curveDegree->addItem("Linear");
	this->curveDegree->addItem("Cubic");
	this->curveName = new ComboBox();
	QPushButton *addB = new QPushButton("+");
	QPushButton *removeB = new QPushButton("-");
	form->addRow("Degree", this->curveDegree);
	form->addRow("Name", createCBField(this->curveName, removeB, addB));
	setFormLayout(form);

	connect(this->curveDegree,
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, &PropertyEditor::setCurveDegree);
	connect(addB, &QPushButton::clicked,
		this, QOverload<>::of(&PropertyEditor::addCurve));
	connect(removeB, &QPushButton::clicked,
		this, &PropertyEditor::removeCurve);
	connect(this->curveName,
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, &PropertyEditor::selectCurve);
	connect(this->curveName->lineEdit(), &QLineEdit::editingFinished,
		this, &PropertyEditor::renameCurve);

	this->curveEditor->setUpdateFunction([&] (pg::Spline spline) {
		createCurveCommand();
		Plant *plant = this->editor->getPlant();
		unsigned index = this->curveName->currentIndex();
		Curve curve = plant->getCurve(index);
		curve.setSpline(spline);
		plant->updateCurve(curve, index);
		this->editor->change();
	});
}

void PropertyEditor::setCurveDegree(int index)
{
	if (!this->editor->getScene()->updating)
		this->curveEditor->setDegree(index);
}

void PropertyEditor::addCurve()
{
	if (!this->editor->getScene()->updating) {
		Curve curve;
		QString name = createUniqueName("Curve ", this->curveName);
		curve.setName(name.toStdString());
		curve.setSpline(pg::Spline(0));
		createCurveCommand();
		addCurve(curve);
		int index = this->curveName->findText(name);
		this->curveName->setCurrentIndex(index);
		selectCurve();
	}
}

void PropertyEditor::addCurve(Curve curve)
{
	Plant *plant = this->editor->getPlant();
	plant->addCurve(curve);
	QString name = QString::fromStdString(curve.getName());
	addCurveName(name);
}

void PropertyEditor::setCurves()
{
	int index = this->curveName->currentIndex();
	if (index >= 0) {
		this->curveName->blockSignals(true);
		this->curveName->clear();
		this->cv[RadiusCurve]->blockSignals(true);
		this->cv[RadiusCurve]->clear();
		Plant *plant = this->editor->getPlant();
		for (const Curve &curve : plant->getCurves()) {
			QString name = QString::fromStdString(curve.getName());
			this->curveName->addItem(name);
			this->cv[RadiusCurve]->addItem(name);
		}
		if (index >= this->curveName->count())
			index = this->curveName->count() - 1;
		this->curveName->setCurrentIndex(index);
		this->curveName->blockSignals(false);
		this->cv[RadiusCurve]->blockSignals(false);
		selectCurve();
	}
}

void PropertyEditor::addCurveName(QString name)
{
	this->curveName->addItem(name);
	this->cv[RadiusCurve]->blockSignals(true);
	this->cv[RadiusCurve]->addItem(name);
	this->cv[RadiusCurve]->blockSignals(false);
}

void PropertyEditor::selectCurve()
{
	this->curveEditor->clear();
	if (this->curveName->count()) {
		Plant *plant = this->editor->getPlant();
		int index = this->curveName->currentIndex();
		Curve curve = plant->getCurve(index);
		this->curveEditor->setSpline(curve.getSpline());
		int degree = curve.getSpline().getDegree();
		this->curveDegree->setCurrentIndex(degree == 3 ? 1 : 0);
	}
}

void PropertyEditor::renameCurve()
{
	unsigned index = this->curveName->currentIndex();
	QString name = this->curveName->itemText(index);
	Plant *plant = this->editor->getPlant();
	Curve curve = plant->getCurve(index);
	curve.setName(name.toStdString());
	createCurveCommand();
	plant->updateCurve(curve, index);

	this->cv[RadiusCurve]->setItemText(index, name);
}

void PropertyEditor::removeCurve()
{
	if (this->curveName->count() > 1) {
		createCurveCommand();
		int index = this->curveName->currentIndex();
		QString name = this->curveName->currentText();
		Plant *plant = this->editor->getPlant();
		plant->removeCurve(index);
		this->curveName->removeItem(index);
		selectCurve();
		this->editor->change();

		int ci = this->cv[RadiusCurve]->currentIndex();
		this->cv[RadiusCurve]->blockSignals(true);
		this->cv[RadiusCurve]->setCurrentIndex((index != ci) * index);
		this->cv[RadiusCurve]->removeItem(index);
		this->cv[RadiusCurve]->blockSignals(false);
	}
}

void PropertyEditor::clearCurves()
{
	Plant *plant = this->editor->getPlant();
	int count = this->curveName->count();
	while (count > 0)
		plant->removeCurve(count--);
	this->curveName->clear();
	this->curveEditor->clear();
}

void PropertyEditor::createCurveCommand()
{
	History *history = this->editor->getHistory();
	const Command *command = history->peak();
	bool undefined = !this->saveCurve || !command;
	if (undefined || command->getTime() != this->saveCurve->getTime()) {
		pg::Scene *scene = this->editor->getScene();
		this->saveCurve = new SaveCurve(scene);
		history->add(this->saveCurve);
	}
}

void PropertyEditor::createMaterialInterface(QBoxLayout *layout)
{
	QGroupBox *materialGroup = createGroup("Material");
	layout->addWidget(materialGroup);
	QBoxLayout *vlayout = new QVBoxLayout(materialGroup);
	vlayout->setMargin(0);
	vlayout->setSpacing(0);
	this->materialViewer = new MaterialViewer(this->shared, this);
	vlayout->addWidget(this->materialViewer);
	QFormLayout *form = createForm(vlayout);
	this->materialName = new ComboBox();
	QPushButton *ab = new QPushButton("+");
	QPushButton *rb = new QPushButton("-");
	form->addRow("Name", createCBField(this->materialName, rb, ab));

	connect(this->materialName,
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, &PropertyEditor::selectMaterial);
	connect(this->materialName->lineEdit(), &QLineEdit::editingFinished,
		this, &PropertyEditor::renameMaterial);
	connect(ab, &QPushButton::clicked,
		this, &PropertyEditor::addMaterial);
	connect(rb, &QPushButton::clicked,
		this, &PropertyEditor::removeMaterial);

	QWidget *fields[Material::MapQuantity];
	for (int i = 0; i < Material::MapQuantity; i++) {
		rb = new QPushButton("-");
		ab = new QPushButton("+");
		this->materialFile[i] = new QLineEdit(this);
		fields[i] = createLEField(this->materialFile[i], rb, ab);
		connect(ab, &QPushButton::clicked,
			this, [this, i]() {this->openMaterialFile(i);});
		connect(rb, &QPushButton::clicked,
			this, [this, i]() {this->removeMaterialFile(i);});
	}

	for (int i = 0; i < MSize; i++) {
		this->materialValue[i] = new DoubleSpinBox(this);
		this->materialValue[i]->setSingleStep(0.001);
		this->materialValue[i]->setDecimals(3);
		connect(this->materialValue[i],
			QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			this, &PropertyEditor::changeMaterial);
	}

	form->addRow("Albedo", fields[Material::Albedo]);
	form->addRow("Opacity", fields[Material::Opacity]);
	form->addRow("Normal", fields[Material::Normal]);
	form->addRow("Specular", fields[Material::Specular]);
	form->addRow("Shininess", this->materialValue[Shininess]);
	this->materialValue[Shininess]->setSingleStep(1);
	this->materialValue[Shininess]->setDecimals(1);
	form->addRow("Ambient.R", this->materialValue[AmbientR]);
	form->addRow("Ambient.G", this->materialValue[AmbientG]);
	form->addRow("Ambient.B", this->materialValue[AmbientB]);
	setFormLayout(form);
}

void PropertyEditor::openMaterialFile(int index)
{
	int selection = this->materialName->currentIndex();
	ShaderParams params = this->shared->getMaterial(selection);
	QString filename = QFileDialog::getOpenFileName(this, "Open File", "",
		"All Files (*);;PNG (*.png);;JPEG (*.jpg);;SVG (*.svg)");

	if (!filename.isNull() || !filename.isEmpty()) {
		QString label = filename.split("/").back();
		this->materialFile[index]->setText(label);
		if (params.loadTexture(index, filename))
			updateMaterial(params, selection);
	}
}

void PropertyEditor::removeMaterialFile(int index)
{
	int selection = this->materialName->currentIndex();
	ShaderParams params = this->shared->getMaterial(selection);
	params.removeTexture(index);
	this->materialFile[index]->clear();
	updateMaterial(params, selection);
}

void PropertyEditor::selectMaterial(int index)
{
	if (this->materialName->count()) {
		ShaderParams params = this->shared->getMaterial(index);
		Material m = params.getMaterial();
		for (int i = 0; i < Material::MapQuantity; i++) {
			QString file = QString::fromStdString(m.getTexture(i));
			QString label = file.split("/").back();
			this->materialFile[i]->setText(label);
		}
		this->materialValue[Shininess]->setValue(m.getShininess());
		this->materialValue[AmbientR]->setValue(m.getAmbient().x);
		this->materialValue[AmbientG]->setValue(m.getAmbient().y);
		this->materialValue[AmbientB]->setValue(m.getAmbient().z);
		this->materialViewer->updateMaterial(index);
	}
}

void PropertyEditor::updateMaterial(ShaderParams params, unsigned index)
{
	this->shared->updateMaterial(params, index);
	this->editor->getPlant()->updateMaterial(params.getMaterial(), index);
	this->editor->change();
	this->materialViewer->updateMaterial(index);
}

void PropertyEditor::changeMaterial()
{
	unsigned index = this->materialName->currentIndex();
	ShaderParams params = this->shared->getMaterial(index);
	Material material = params.getMaterial();
	material.setShininess(this->materialValue[Shininess]->value());
	material.setAmbient(Vec3(
		this->materialValue[AmbientR]->value(),
		this->materialValue[AmbientG]->value(),
		this->materialValue[AmbientB]->value()));
	params.swapMaterial(material);
	updateMaterial(params, index);
}

void PropertyEditor::renameMaterial()
{
	unsigned index = this->materialName->currentIndex();
	QString name = this->materialName->itemText(index);
	ShaderParams params = this->shared->getMaterial(index);
	params.setName(name.toStdString());
	updateMaterial(params, index);

	this->cv[StemMaterial]->setItemText(index, name);
	this->cv[CapMaterial]->setItemText(index, name);
	this->cv[LeafMaterial]->setItemText(index, name);
}

void PropertyEditor::addMaterial()
{
	ShaderParams params;
	QString name = createUniqueName("Material ", this->materialName);
	params.setName(name.toStdString());
	unsigned index = this->shared->addMaterial(params);
	this->editor->getPlant()->addMaterial(params.getMaterial());
	this->materialViewer->updateMaterial(index);
	addMaterialName(name);
	index = this->materialName->findText(name);
	this->materialName->setCurrentIndex(index);
	for (int i = 0; i < Material::MapQuantity; i++)
		this->materialFile[i]->clear();
}

void PropertyEditor::addMaterialName(QString name)
{
	this->materialName->addItem(name);
	blockSignals(true);
	this->cv[StemMaterial]->addItem(name);
	this->cv[CapMaterial]->addItem(name);
	this->cv[LeafMaterial]->addItem(name);
	blockSignals(false);
}

void PropertyEditor::removeMaterial()
{
	if (this->materialName->count() > 1) {
		int index = this->materialName->currentIndex();
		this->materialName->removeItem(index);
		this->shared->removeMaterial(index);
		this->editor->getPlant()->removeMaterial(index);
		this->editor->change();
		selectMaterial(this->materialName->currentIndex());

		blockSignals(true);
		int si = this->cv[StemMaterial]->currentIndex();
		int ci = this->cv[CapMaterial]->currentIndex();
		int li = this->cv[LeafMaterial]->currentIndex();
		this->cv[StemMaterial]->setCurrentIndex((index != si) * index);
		this->cv[CapMaterial]->setCurrentIndex((index != ci) * index);
		this->cv[LeafMaterial]->setCurrentIndex((index != li) * index);
		this->cv[StemMaterial]->removeItem(index);
		this->cv[CapMaterial]->removeItem(index);
		this->cv[LeafMaterial]->removeItem(index);
		blockSignals(false);
	}
}

void PropertyEditor::createMeshInterface(QBoxLayout *layout)
{
	QGroupBox *meshGroup = createGroup("Mesh");
	layout->addWidget(meshGroup);
	QBoxLayout *vlayout = new QVBoxLayout(meshGroup);
	vlayout->setMargin(0);
	vlayout->setSpacing(0);
	this->meshViewer = new MeshViewer(shared, meshGroup);
	vlayout->addWidget(this->meshViewer);
	QFormLayout *form = createForm(vlayout);

	this->meshName = new ComboBox();
	QPushButton *ab = new QPushButton("+");
	QPushButton *rb = new QPushButton("-");
	form->addRow("Name", createCBField(this->meshName, rb, ab));
	QPushButton *button[4];
	button[0] = new QPushButton("Custom", this);
	button[0]->setFixedHeight(UI_FIELD_HEIGHT);
	button[1] = new QPushButton("Plane", this);
	button[1]->setFixedHeight(UI_FIELD_HEIGHT);
	button[2] = new QPushButton("Perpendicular Planes", this);
	button[2]->setFixedHeight(UI_FIELD_HEIGHT);
	button[3] = new QPushButton("Empty", this);
	button[3]->setFixedHeight(UI_FIELD_HEIGHT);
	form->addRow("File", button[0]);
	form->addRow("Preset", button[1]);
	form->addRow("Preset", button[2]);
	form->addRow("Preset", button[3]);
	setFormLayout(form);

	connect(ab, &QPushButton::clicked, this, &PropertyEditor::addMesh);
	connect(rb, &QPushButton::clicked, this, &PropertyEditor::removeMesh);
	connect(this->meshName,
		QOverload<int>::of(&ComboBox::currentIndexChanged),
		this, &PropertyEditor::selectMesh);
	connect(this->meshName->lineEdit(), &QLineEdit::editingFinished,
		this, &PropertyEditor::renameMesh);

	connect(button[0], &QPushButton::clicked,
		this, &PropertyEditor::loadCustomMesh);
	connect(button[1], &QPushButton::clicked,
		this, &PropertyEditor::loadPlaneMesh);
	connect(button[2], &QPushButton::clicked,
		this, &PropertyEditor::loadPerpPlaneMesh);
	connect(button[3], &QPushButton::clicked,
		this, &PropertyEditor::loadEmptyMesh);
}

void PropertyEditor::addMesh()
{
	pg::Geometry geom;
	QString name = createUniqueName("Mesh ", this->meshName);
	geom.setName(name.toStdString());
	geom.setPlane();
	Plant *plant = this->editor->getPlant();
	plant->addLeafMesh(geom);
	addMeshName(name);
	this->meshViewer->updateMesh(geom);
	this->meshName->setCurrentIndex(this->meshName->findText(name));
}

void PropertyEditor::addMeshName(QString name)
{
	this->meshName->addItem(name);
	this->cv[LeafMesh]->addItem(name);
}

void PropertyEditor::removeMesh()
{
	if (this->meshName->count() > 1) {
		int index = this->meshName->currentIndex();
		QString name = this->meshName->currentText();
		Plant *plant = this->editor->getPlant();
		plant->removeLeafMesh(index);
		this->meshName->removeItem(index);
		selectMesh();
		this->editor->change();

		int li = this->cv[LeafMesh]->currentIndex();
		this->cv[LeafMesh]->blockSignals(true);
		this->cv[LeafMesh]->setCurrentIndex((index != li) * index);
		this->cv[LeafMesh]->removeItem(index);
		this->cv[LeafMesh]->blockSignals(false);
	}
}

void PropertyEditor::selectMesh()
{
	if (this->meshName->count()) {
		Plant *plant = editor->getPlant();
		int index = this->meshName->currentIndex();
		pg::Geometry geom = plant->getLeafMesh(index);
		this->meshViewer->updateMesh(geom);
	}
}

void PropertyEditor::renameMesh()
{
	int index = this->meshName->currentIndex();
	QString name = this->meshName->itemText(index);
	Plant *plant = this->editor->getPlant();
	pg::Geometry geom = plant->getLeafMesh(index);
	geom.setName(name.toStdString());
	plant->updateLeafMesh(geom, index);

	this->cv[LeafMesh]->setItemText(index, name);
}

void PropertyEditor::loadCustomMesh()
{
	if (this->meshName->count() > 0) {
		QString filename = QFileDialog::getOpenFileName(this,
			"Open File", "", "Wavefront OBJ (*.obj)");

		if (!filename.isNull()) {
			Plant *plant = this->editor->getPlant();
			int index = this->meshName->currentIndex();
			pg::Geometry geom = plant->getLeafMesh(index);
			pg::Wavefront obj;
			obj.importFile(filename.toStdString().c_str(), &geom);
			modifyMesh(geom, index);
		}
	}
}

void PropertyEditor::loadPlaneMesh()
{
	if (this->meshName->count() > 0) {
		Plant *plant = editor->getPlant();
		int index = this->meshName->currentIndex();
		pg::Geometry geom = plant->getLeafMesh(index);
		geom.setPlane();
		modifyMesh(geom, index);
	}
}

void PropertyEditor::loadPerpPlaneMesh()
{
	if (this->meshName->count() > 0) {
		Plant *plant = this->editor->getPlant();
		int index = this->meshName->currentIndex();
		pg::Geometry geom = plant->getLeafMesh(index);
		geom.setPerpendicularPlanes();
		modifyMesh(geom, index);
	}
}

void PropertyEditor::loadEmptyMesh()
{
	if (this->meshName->count() > 0) {
		Plant *plant = this->editor->getPlant();
		int index = this->meshName->currentIndex();
		pg::Geometry geom = plant->getLeafMesh(index);
		geom.clear();
		modifyMesh(geom, index);
	}
}

void PropertyEditor::modifyMesh(pg::Geometry &geom, int index)
{
	Plant *plant = this->editor->getPlant();
	plant->updateLeafMesh(geom, index);
	this->meshViewer->updateMesh(geom);
	this->editor->change();
}

void PropertyEditor::createWindInterface(QBoxLayout *layout)
{
	QGroupBox *windGroup = createGroup("Wind");
	layout->addWidget(windGroup);
	QFormLayout *form = createForm(windGroup);

	for (int i = 0; i < WDSize; i++) {
		this->wdv[i] = new DoubleSpinBox(this);
		this->wdv[i]->setDecimals(3);
		this->wdv[i]->setSingleStep(0.01);
		this->wdv[i]->setRange(-1000.0f, 1000.0f);
		connect(this->wdv[i],
			QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			this, &PropertyEditor::changeWind);
	}
	for (int i = 0; i < WISize; i++) {
		this->wiv[i] = new SpinBox(this);
		connect(this->wiv[i],
			QOverload<int>::of(&QSpinBox::valueChanged),
			this, &PropertyEditor::changeWind);
	}

	this->wiv[Seed]->setSingleStep(1);
	this->wiv[Seed]->setRange(
		std::numeric_limits<int>::min(),
		std::numeric_limits<int>::max());
	form->addRow("Seed", this->wiv[Seed]);
	form->addRow("Direction.X", this->wdv[DirectionX]);
	form->addRow("Direction.Y", this->wdv[DirectionY]);
	form->addRow("Direction.Z", this->wdv[DirectionZ]);
	form->addRow("Radius Threshold", this->wdv[Threshold]);
	this->wdv[Resistance]->setSingleStep(0.1);
	form->addRow("Resistance", this->wdv[Resistance]);
	this->wiv[TimeStep]->setValue(30);
	form->addRow("Time Step", this->wiv[TimeStep]);
	this->wiv[FrameCount]->setValue(21);
	form->addRow("Frames", this->wiv[FrameCount]);
	setFormLayout(form);
}

void PropertyEditor::changeWind()
{
	pg::Wind *wind = &this->editor->getScene()->wind;
	Vec3 direction;
	direction.x = this->wdv[DirectionX]->value();
	direction.y = this->wdv[DirectionY]->value();
	direction.z = this->wdv[DirectionZ]->value();
	wind->setDirection(direction);
	wind->setSeed(this->wiv[Seed]->value());
	wind->setResistance(this->wdv[Resistance]->value());
	wind->setThreshold(this->wdv[Threshold]->value());
	wind->setFrameCount(this->wiv[FrameCount]->value());
	wind->setTimeStep(this->wiv[TimeStep]->value());
	this->editor->changeWind();
}

void PropertyEditor::setWindFields()
{
	blockWindSignals(true);
	const pg::Wind *wind = &this->editor->getScene()->wind;
	this->wdv[DirectionX]->setValue(wind->getDirection().x);
	this->wdv[DirectionY]->setValue(wind->getDirection().y);
	this->wdv[DirectionZ]->setValue(wind->getDirection().z);
	this->wdv[Threshold]->setValue(wind->getThreshold());
	this->wdv[Resistance]->setValue(wind->getResistance());
	this->wiv[Seed]->setValue(wind->getSeed());
	this->wiv[FrameCount]->setValue(wind->getFrameCount());
	this->wiv[TimeStep]->setValue(wind->getTimeStep());
	blockWindSignals(false);
}

void PropertyEditor::blockWindSignals(bool block)
{
	for (int i = 0; i < WDSize; i++)
		this->wdv[i]->blockSignals(block);
	for (int i = 0; i < WISize; i++)
		this->wiv[i]->blockSignals(block);
}

void PropertyEditor::populate()
{
	Plant *plant = this->editor->getPlant();
	for (const Curve &curve : plant->getCurves())
		addCurveName(QString::fromStdString(curve.getName()));
	for (const Material &material : plant->getMaterials())
		addMaterialName(QString::fromStdString(material.getName()));
	for (const pg::Geometry &geometry : plant->getLeafMeshes())
		addMeshName(QString::fromStdString(geometry.getName()));
	setWindFields();
}

void PropertyEditor::clear()
{
	blockSignals(true);
	this->cv[StemMaterial]->clear();
	this->cv[CapMaterial]->clear();
	this->cv[RadiusCurve]->clear();
	this->cv[LeafMaterial]->clear();
	this->cv[LeafMesh]->clear();
	blockSignals(false);
	this->curveName->clear();
	this->materialName->clear();
	this->meshName->clear();
}
