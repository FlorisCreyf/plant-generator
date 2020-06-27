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

#ifndef PROPERTY_EDITOR_H
#define PROPERTY_EDITOR_H

#include "editor.h"
#include "stem_editor.h"
#include "leaf_editor.h"
#include <QtWidgets>

class PropertyEditor : public QWidget {
	Q_OBJECT

public:
	PropertyEditor(
		SharedResources *shared, Editor *editor, QWidget *parent);
	QSize sizeHint() const;

public slots:
	void setFields();
	void addCurve(pg::Curve curve);
	void updateCurve(pg::Curve curve, unsigned index);
	void removeCurve(unsigned index);
	void addMaterial(ShaderParams params);
	void updateMaterials();
	void removeMaterial(unsigned index);
	void addMesh(pg::Geometry geom);
	void updateMesh(pg::Geometry geom, unsigned index);
	void removeMesh(unsigned index);

signals:
	void isEnabled(bool enabled);
	void radiusCurveChanged(std::vector<pg::Vec3>);

private:
	Editor *editor;
	StemEditor *stemEditor;
	LeafEditor *leafEditor;

	void initProperties();
	void setValueWidths(QFormLayout *);
};

#endif /* PROPERTY_EDITOR_H */
