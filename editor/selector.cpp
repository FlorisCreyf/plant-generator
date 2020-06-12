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

#include "selector.h"

using pg::Mesh;
using pg::Path;
using pg::Spline;
using pg::Stem;
using pg::Vec3;
using std::pair;

Selector::Selector(const Camera *camera)
{
	this->camera = camera;
}

void Selector::select(
	const QMouseEvent *event, const Mesh *mesh, Selection *selection)
{
	if (!selectPoint(event, selection))
		selectMesh(event, mesh, selection);
}

bool Selector::selectPoint(const QMouseEvent *event, Selection *selection)
{
	int point = -1;

	/* Try to select a point from an already selected stem. */
	for (auto &instance : selection->getStemInstances()) {
		Stem *stem = instance.first;
		PointSelection &ps = instance.second;
		Spline spline = stem->getPath().getSpline();
		Vec3 location = stem->getLocation();
		point = selectPoint(event, spline, location, &ps);
		if (point >= 0) {
			if (!(event->modifiers() & Qt::ControlModifier))
				selection->clear();
			selection->addStem(stem, ps);
			break;
		}
	}

	return point >= 0;
}

void Selector::selectMesh(
	const QMouseEvent *event, const Mesh *mesh, Selection *selection)
{
	bool ctrl = event->modifiers() & Qt::ControlModifier;
	QPoint point = event->pos();
	pg::Ray ray = this->camera->getRay(point.x(), point.y());
	Stem *root = selection->getPlant()->getRoot();
	pair<float, Stem *> stemPair = getStem(ray, root);
	pair<float, pg::Segment> leafPair = getLeaf(ray, mesh);

	/* Remove previous selections if no modifier key is pressed. */
	if (!ctrl)
		selection->clear();

	if (stemPair.second && stemPair.first < leafPair.first) {
		if (!selection->removeStem(stemPair.second))
			selection->addStem(stemPair.second);
	} else if (leafPair.second.stem && leafPair.first < stemPair.first) {
		unsigned leaf = leafPair.second.leafIndex;
		Stem *stem = leafPair.second.stem;
		if (!selection->removeLeaf(stem, leaf))
			selection->addLeaf(stem, leaf);
	} else if (!ctrl)
		selection->clear();
}

/** Performs cylinder intersection tests to determine which stem was clicked
on. A stem and the distance to its intersection is returned. */
pair<float, Stem *> Selector::getStem(pg::Ray &ray, Stem *stem)
{
	float max = std::numeric_limits<float>::max();
	pair<float, Stem *> selection1(max, nullptr);
	pair<float, Stem *> selection2(max, nullptr);

	if (stem != nullptr) {
		Path path = stem->getPath();
		for (size_t i = 0, j = 1; i < path.getSize()-1; i++, j++) {
			Vec3 direction = path.getDirection(i);
			Vec3 line[2] = {path.get(i), path.get(j)};
			line[0] = line[0] + stem->getLocation();
			line[1] = line[1] + stem->getLocation();
			float length = pg::magnitude(line[1] - line[0]);
			float r[2] = {path.getRadius(i), path.getRadius(j)};
			float t = pg::intersectsTaperedCylinder(
				ray, line[0], direction, length, r[0], r[1]);
			if (t > 0 && selection1.first > t) {
				selection1.first = t;
				selection1.second = stem;
			}
		}

		selection2 = getStem(ray, stem->getChild());
		if (selection2.second && selection2.first < selection1.first)
			selection1 = selection2;
		selection2 = getStem(ray, stem->getSibling());
		if (selection2.second && selection2.first < selection1.first)
			selection1 = selection2;
	}

	return selection1;
}

pair<float, pg::Segment> Selector::getLeaf(pg::Ray ray, const Mesh *mesh)
{
	unsigned indexOffset = 0;
	unsigned vertexOffset = 0;

	pair<float, pg::Segment> selection;
	selection.first = std::numeric_limits<float>::max();
	selection.second.stem = nullptr;

	for (size_t m = 0; m < mesh->getMeshCount(); m++) {
		auto vertices = mesh->getVertices(m);
		auto indices = mesh->getIndices(m);
		auto leaves = mesh->getLeaves(m);
		for (auto pair : leaves) {
			pg::Segment segment = pair.second;
			segment.indexStart -= indexOffset;

			size_t len = segment.indexStart + segment.indexCount;
			for (size_t i = segment.indexStart; i < len; i += 3) {
				unsigned triangle[3];
				triangle[0] = (*indices)[i] - vertexOffset;
				triangle[1] = (*indices)[i+1] - vertexOffset;
				triangle[2] = (*indices)[i+2] - vertexOffset;

				Vec3 v1 = (*vertices)[triangle[0]].position;
				Vec3 v2 = (*vertices)[triangle[1]].position;
				Vec3 v3 = (*vertices)[triangle[2]].position;

				float minDistance = selection.first;
				float distance = pg::intersectsTriangle(
					ray, v1, v2, v3);
				if (distance > 0 && distance < minDistance) {
					selection.first = distance;
					selection.second = segment;
				}
			}
		}
		indexOffset += indices->size();
		vertexOffset += vertices->size();
	}
	return selection;
}

int Selector::selectPoint(const QMouseEvent *event, const Spline &spline,
	Vec3 location, PointSelection *selection)
{
	bool ctrl = event->modifiers() & Qt::ControlModifier;
	auto controls = spline.getControls();
	int size = controls.size();
	int degree = spline.getDegree();
	int selectedPoint = -1;

	/* Find what point is clicked on. */
	for (int i = 0; i < size; i++) {
		Vec3 point = controls[i] + location;
		point = this->camera->toScreenSpace(point);
		float sx = std::pow(point.x - event->pos().x(), 2);
		float sy = std::pow(point.y - event->pos().y(), 2);

		if (degree == 3 && i % 3 == 0) {
			if (std::sqrt(sx + sy) < 5) {
				selectedPoint = i;
				break;
			}
		} else if (std::sqrt(sx + sy) < 10) {
			if (degree == 3 && selectedPoint % 3 != 0)
				selectedPoint = i;
			else {
				selectedPoint = i;
				break;
			}
		}
	}

	/* Remove previous selections if no modifier key is pressed. */
	if (!ctrl)
		selection->clear();

	if (selectedPoint >= 0) {
		/* Remove the point from the selection if it is already
		selected. */
		if (!selection->removePoint(selectedPoint))
			selection->addPoint(selectedPoint);
	} else if (!ctrl)
		/* Remove the entire selection if nothing was clicked on. */
		selection->clear();

	return selectedPoint;
}
