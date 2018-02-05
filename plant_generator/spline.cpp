/* Copyright 2017 Floris Creyf
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "spline.h"
#include "math/curve.h"

void pg::Spline::setControls(std::vector<pg::Vec3> controls)
{
	this->controls = controls;
}

std::vector<pg::Vec3> pg::Spline::getControls() const
{
	return controls;
}

int pg::Spline::getCurveCount() const
{
	/* minus one if there are no overlapping points */
	if (controls.size() == 0)
		return 0;
	else
		return (controls.size() - 1) / degree;
}

void pg::Spline::setDegree(int degree)
{
	this->degree = degree;
}

int pg::Spline::getDegree() const
{
	return degree;
}

pg::Vec3 pg::Spline::getPoint(float t) const
{
	Vec3 point;
	for (size_t i = 0; i < controls.size()-degree; i += degree) {
		if (controls[i].x <= t && controls[i+degree].x >= t) {
			t -= controls[i].x;
			t /= (controls[i+degree].x - controls[i].x);
			point = getBezier(t, &controls[i], (degree + 1));
			break;
		}
	}
	return point;
}

pg::Vec3 pg::Spline::getPoint(int curve, float t) const
{
	int index = degree * curve;
	return getBezier(t, &controls[index], (degree + 1));
}

int pg::Spline::insert(unsigned index, pg::Vec3 point)
{
	Vec3 curve[3];
	int center;

	switch (degree) {
	case 1:
		controls.insert(controls.begin() + index + 1, point);
		center = index + 1;
		break;
	case 3:
		switch(index % 3) {
		case 0:
			if (index == controls.size() - 1) {
				Vec3 d = controls[index-1] - controls[index];
				curve[0] = -1.0f * d + point;
				curve[1] = d + point;
				curve[2] = point;
				center = index + 3;
				++index;
			} else {
				Vec3 d = controls[index-1] - controls[index];
				curve[1] = point;
				d = controls[index+1] - controls[index];
				curve[2] = d + point;
				center = index + 3;
				index += 2;
			}
			break;
		case 1:
			if (index == 1) {
				Vec3 d = controls[index] - controls[index-1];
				curve[0] = -1.0f * d + point;
				curve[1] = point;
				curve[2] = d + point;
				center = index + 2;
				++index;
			} else {
				Vec3 d = controls[index-1] - controls[index];
				curve[0] = d + point;
				curve[1] = point;
				d = controls[index-1] - controls[index-2];
				curve[2] = d + point;
				center = index + 2;
				++index;
			}
			break;
		case 2:
			if (index == controls.size() - 2) {
				Vec3 d = controls[index+1] - controls[index];
				curve[0] = -1.0f * d + point;
				curve[1] = point;
				curve[2] = d + point;
				center = index + 1;
			} else {
				Vec3 d = controls[index] - controls[index+1];
				curve[0] = d + point;
				curve[1] = point;
				d = controls[index+2] - controls[index+1];
				curve[2] = d + point;
				center = index + 1;
			}
			break;
		}

		controls.insert(controls.begin() + index, curve, &curve[3]);
		break;
	}

	return center;
}

void pg::Spline::remove(unsigned index)
{
	std::vector<pg::Vec3> copy;
	switch (degree) {
	case 1:
		controls.erase(controls.begin() + index);
		break;
	case 3:
		if (controls.size() == 4) {
			controls.clear();
		} else if (index < 2) {
			controls.erase(controls.begin(), controls.begin() + 2);
		} else if (index > controls.size() - 3) {
			controls.erase(controls.end() - 3, controls.end());
		} else {
			switch(index % 3) {
			case 0:
				index -= 1;
				break;
			case 1:
				index -= 2;
				break;
			}
			auto first = controls.begin() + index;
			auto last = controls.begin() + index + 3;
			controls.erase(first, last);
		}
		break;
	}
}

int pg::Spline::adjust(int degree, int index)
{
	switch (degree) {
	case 1:
		if (this->degree != degree) {
			switch (index % 3) {
			case 1:
				index--;
				break;
			case 2:
				index++;
				break;
			}
			index /= 3;
		}

		for (size_t i = 0; i + 3 < controls.size(); i++)
			controls.erase(controls.begin() + 1 + i,
				controls.begin() + 3 + i);
		break;
	case 3:
		if (this->degree != degree)
			index *= 3;

		{
			Vec3 direction = normalize(controls[1] - controls[0]);
			float t = magnitude(controls[1] - controls[0]) / 4.0f;
			Vec3 point = controls[0] + t * direction;
			controls.insert(controls.begin() + 1, point);
		}

		for (size_t i = 2; i < controls.size()-1; i += 3) {
			Vec3 d1 = controls[i-2] - controls[i];
			Vec3 d2 = controls[i] - controls[i+1];
			float t1 = magnitude(d1) / 4.0f;
			float t2 = -magnitude(d2) / 4.0f;
			Vec3 direction = normalize(d1 + d2);

			Vec3 points[2];
			points[0] = controls[i] + t1 * direction;
			points[1] = controls[i] + t2 * direction;
			controls.insert(controls.begin() + i + 1, points[1]);
			controls.insert(controls.begin() + i, points[0]);
		}

		{
			int l = controls.size() - 1;
			Vec3 direction = normalize(controls[l] - controls[l-1]);
			float t = magnitude(controls[l] - controls[l-2]) / 4.0f;
			Vec3 point = controls[l] - t * direction;
			controls.insert(controls.end() - 1, point);
		}
		break;
	}

	this->degree = degree;
	return index;
}

void pg::Spline::move(unsigned index, pg::Vec3 location)
{
	if (degree == 3) {
		switch (index % 3) {
		case 0:
			if (index != 0) {
				controls[index-1] -= controls[index];
				controls[index-1] += location;
			}
			if (index != controls.size() - 1) {
				controls[index+1] -= controls[index];
				controls[index+1] += location;
			}
			controls[index] = location;
			break;
		case 1:
			if (index != 1) {
				Vec3 d = controls[index-1] - controls[index-2];
				float m = magnitude(d);
				if (m == 0.0f)
					m = 1.0f;
				d = controls[index-1] - controls[index];
				/* Arbitrary default value because zero vectors
				 * cannot be normalized. */
				if (isZero(d))
					d.z = 1.0f;
				d = m * normalize(d) + controls[index-1];
				controls[index-2] = d;
			}
			controls[index] = location;
			break;
		case 2:
			if (index != controls.size() - 2) {
				Vec3 d = controls[index+2] - controls[index+1];
				float m = magnitude(d);
				if (m == 0.0f)
					m = 1.0f;
				d = controls[index+1] - controls[index];
				if (isZero(d))
					d.z = 1.0f;
				d = m * normalize(d) + controls[index+1];
				controls[index+2] = d;
			}
			controls[index] = location;
			break;
		}
	} else
		controls[index] = location;
}
