#include "treemaker.h"
#include "tree_impl.h"
#include <time.h>
#include <stdlib.h> 

using namespace treemaker;

treemaker::Tree::Tree()
{
	d = new TreeImpl();
}

treemaker::Tree::~Tree()
{
	delete d;
}

void treemaker::Tree::generateTree()
{
	d->procGenerator.generateTree();
}

bool treemaker::Tree::generateMesh()
{
	return d->meshGenerator.generate(d->getRoot());
}

void treemaker::Tree::setMaxStemDepth(unsigned depth)
{
	d->maxStemDepth = depth;
}

unsigned treemaker::Tree::getMaxStemDepth()
{
	return d->maxStemDepth;
}

void treemaker::Tree::setBaseLength(unsigned stem, float length)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr) {
		s->baseLength = length;
		d->stemModifier.updateBaseLength(s);
		d->changeMode(s);
	}
}

float treemaker::Tree::getBaseLength(unsigned stem)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr)
		return s->baseLength;
	else
		return 0.0f;
}

unsigned treemaker::Tree::getStemName(size_t index)
{
	Stem *s = d->findStemByIndex(index);
	if (s == nullptr)
		return (unsigned) - 1;
	else
		return s->getName();
}

void treemaker::Tree::setMode(unsigned stem, Flags mode)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr)
		d->changeMode(s, mode);
}

Flags treemaker::Tree::getMode(unsigned stem)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr)
		return s->mode;
	else
		return Flags::UNDEFINED;
}

void treemaker::Tree::setDistribution(unsigned stem, Flags distribution)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr)
		s->distribution = distribution;
}

Flags treemaker::Tree::getDistribution(unsigned stem)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr)
		return s->distribution;
	else
		return Flags::UNDEFINED;
}

unsigned treemaker::Tree::newStem(unsigned parent, float position)
{
	Stem *s = d->findStem(parent);
	if (s != nullptr) {
		d->changeMode(s, Flags::MANUAL);
		s = s->addLateralStem(d->nameGenerator);
		d->procGenerator.growLateralStem(s, position);
		return s->getName();
	} else
		return 0;
}

void treemaker::Tree::deleteStem(unsigned stem)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr) {
		d->deleteStem(s);
		d->changeMode(s->getParent(), Flags::MANUAL);
	}
}

bool treemaker::Tree::isLateral(unsigned stem)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr)
		return s->isLateral();
	else
		return false;
}

unsigned treemaker::Tree::getParent(unsigned stem)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr && s->getParent() != nullptr)
		return s->getParent()->getName();
	else
		return 0;
}

void treemaker::Tree::setPosition(unsigned stem, float position)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr) {
		s->setPosition(position);
		d->changeMode(s->getParent(), Flags::MANUAL);
	}
}

float treemaker::Tree::getPosition(unsigned stem)
{
	Stem *s = d->findStem(stem);
	return s != nullptr ? s->getPosition() : 0.0f;
}

void treemaker::Tree::setLocation(unsigned stem, Vec3 location)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr && !s->isLateral()) {
		Stem *parent = s->getParent();
		if (parent != nullptr) {
			auto p = parent->getPath();
			auto c = s->getParent()->getPath().getControls();
			c[c.size() - 1] = location - parent->getLocation();
			setPath(parent->getName(), &c[0], c.size());
		}
	}
}

Vec3 treemaker::Tree::getLocation(unsigned stem)
{
	Stem *s = d->findStem(stem);
	Vec3 v = {};
	return s != nullptr ? s->getLocation() : v;
}

void treemaker::Tree::setRadius(unsigned stem, float radius)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr) {
		if (radius >= s->minRadius)
			s->radius = radius;
		else
			s->radius = s->minRadius;
	}
}

float treemaker::Tree::getRadius(unsigned stem)
{
	Stem *s = d->findStem(stem);
	return s != nullptr ? s->radius : 0.0f;
}

void treemaker::Tree::setRadiusCurve(unsigned stem, Vec3 *curve, size_t size)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr) {
		std::vector<Vec3> *c = &s->radiusCurve;
		c->clear();
		c->insert(c->end(), &curve[0], &curve[size]);
	}
}

size_t treemaker::Tree::getRadiusCurveSize(unsigned stem)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr)
		return s->radiusCurve.size();
	else
		return 0;
}

void treemaker::Tree::getRadiusCurve(unsigned stem, Vec3 *curve)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr)
		std::copy(s->radiusCurve.begin(), s->radiusCurve.end(), curve);
}

void treemaker::Tree::setStemDensity(unsigned stem, float density)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr && s->stemDensity != density) {
		s->stemDensity = density;
		d->stemModifier.updateStemDensity(s);
		d->changeMode(s, ASSISTED);
	}
}

float treemaker::Tree::getStemDensity(unsigned stem)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr)
		return s->stemDensity;
	else
		return 0.0f;
}

void treemaker::Tree::setResolution(unsigned stem, int resolution)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr)
		s->setResolution(resolution);
}

int treemaker::Tree::getResolution(unsigned stem)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr)
		return s->getResolution();
	else
		return -1;
}

void treemaker::Tree::setPath(unsigned stem, Vec3 *path, size_t size)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr) {
		float length = s->getPath().getLength() - s->baseLength;
		auto p = s->getPath();
		std::vector<Vec3> c(&path[0], &path[size]);
		for (size_t i = 0; i < c.size(); i++)
			c[i] = c[i] - s->getLocation();
		p.setControls(c);
		s->setPath(p);
		d->stemModifier.updateStemDensity(s);
		d->changeMode(s);
		if (s->mode == MANUAL && s->distribution == DISTRIBUTED)
			d->stemModifier.distribute(s, length);
	}
}

size_t treemaker::Tree::getPathSize(unsigned stem)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr)
		return s->getPath().getControls().size();
	else
		return 0;
}

void treemaker::Tree::getPath(unsigned stem, Vec3 *path)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr) {
		auto p = s->getPath();
		auto controls = p.getControls();
		for (size_t i = 0; i < controls.size(); i++)
			path[i] = controls[i] + s->getLocation();
	}
}

size_t treemaker::Tree::getGeneratedPathSize(unsigned stem)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr)
		return s->getPath().getDivisions();
	else
		return 0;
}

void treemaker::Tree::getGeneratedPath(unsigned stem, Vec3 *path)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr) {
		auto p = s->getPath();
		auto points = p.getPath();
		std::copy(points.begin(), points.end(), path);
		for (size_t i = 0; i < points.size(); i++)
			path[i] = points[i] + s->getLocation();
	}
}

void treemaker::Tree::setGeneratedPathSize(unsigned stem, size_t count)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr) {
		auto p = s->getPath();
		p.setDivisions(count);
		s->setPath(p);
	}
}

const float *treemaker::Tree::getVertices()
{
	return d->meshGenerator.getVertices();
}

size_t treemaker::Tree::getVertexCount()
{
	return d->meshGenerator.getVertexCount();
}

size_t treemaker::Tree::getVertexCapacity()
{
	return d->meshGenerator.getVertexCapacity();
}

const unsigned *treemaker::Tree::getIndices()
{
	return d->meshGenerator.getIndices();
}

size_t treemaker::Tree::getIndexCount()
{
	return d->meshGenerator.getIndexCount();
}

size_t treemaker::Tree::getIndexCapacity()
{
	return d->meshGenerator.getIndexCapacity();
}

Tree::Location treemaker::Tree::getStemLocation(unsigned stem)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr) {
		Location l = {s->indexStart, s->indexCount};
		return l;
	}
	
	Location l = {};
	return l;
}

Aabb treemaker::Tree::getBoundingBox(unsigned stem)
{
	Stem *s = d->findStem(stem);
	if (s != nullptr) {
		const float *vertices = d->meshGenerator.getVertices();
		return createAABB(&vertices[s->vertexStart], s->vertexCount);
	}
	
	Aabb a = {};
	return a;
}
