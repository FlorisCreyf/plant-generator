/* Plant Genererator
 * Copyright (C) 2018  Floris Creyf
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

#include "path.h"

using std::vector;

void Path::setColor(pg::Vec3 curveColor, pg::Vec3 controlColor)
{
	this->curveColor = curveColor;
	this->controlColor = controlColor;
}

void Path::set(const pg::Spline &spline, int resolution)
{
	vector<pg::Vec3> points;
	vector<pg::Vec3> controls = spline.getControls();
	float step = 1.0f/(resolution - 1);
	for (int curve = 0; curve < spline.getCurveCount(); curve++) {
		float t = 0.0f;
		for (int i = 0; i < resolution; i++, t += step)
			points.push_back(spline.getPoint(curve, t));
	}
	set(points, controls, spline.getDegree());
}

void Path::set(const vector<pg::Vec3> &points, const vector<pg::Vec3> &controls,
	int degree)
{
	size_t index = 0;
	path.clear();

	/* curve */
	for (index = 0; index < points.size(); index++) {
		path.addPoint(points[index], curveColor);
		path.addIndex(index);
	}

	/* control lines */
	switch (degree) {
	case 2:
		path.addIndex(Geometry::primitiveReset);
		for (size_t i = 0; i < controls.size(); i++) {
			path.addPoint(controls[i], controlColor);
			path.addIndex(index++);
		}
		break;
	case 3:
		if (controls.size() > 1) {
			size_t i = 0;
			path.addIndex(Geometry::primitiveReset);

			path.addPoint(controls[i++], controlColor);
			path.addIndex(index++);
			path.addPoint(controls[i++], controlColor);
			path.addIndex(index++);
			path.addIndex(Geometry::primitiveReset);

			while (i < controls.size() - 2) {
				path.addPoint(controls[i++], controlColor);
				path.addIndex(index++);
				path.addPoint(controls[i++], controlColor);
				path.addIndex(index++);
				path.addPoint(controls[i++], controlColor);
				path.addIndex(index++);
				path.addIndex(Geometry::primitiveReset);
			}

			path.addPoint(controls[i++], controlColor);
			path.addIndex(index++);
			path.addPoint(controls[i++], controlColor);
			path.addIndex(index);
		}
		break;
	default:
		break;
	}

	lineSegment = path.getSegment();

	/* control points */
	for (size_t i = 0; i < controls.size(); i++)
		path.addPoint(controls[i], controlColor);

	pointSegment.pstart = lineSegment.pcount;
	pointSegment.pcount = controls.size();
}

const Geometry *Path::getGeometry()
{
	return &path;
}

Geometry::Segment Path::getLineSegment()
{
	return lineSegment;
}

Geometry::Segment Path::getPointSegment()
{
	return pointSegment;
}
