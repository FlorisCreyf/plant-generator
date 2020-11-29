#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../plant_generator/wind.h"

using namespace pg;
namespace bt = boost::unit_test;

BOOST_AUTO_TEST_SUITE(wind)

Path createPath()
{
	Path path;
	path.setDivisions(2);
	Spline spline;
	spline.setDegree(3);
	spline.addControl(Vec3(0.0f, 0.0f, 0.0f));
	for (int i = 0; i < 4; i++) {
		spline.addControl(Vec3(i, i+0.5f, i));
		spline.addControl(Vec3(i, i+0.5f, i));
		spline.addControl(Vec3(i, i+1.0f, i));
	}
	path.setSpline(spline);
	return path;
}

void addStem(Plant &plant, Stem *root, Path &path, float position)
{
	Stem *stem = plant.addStem(root);
	stem->setPath(path);
	stem->setDistance(position);
}

void validateJoints(Stem *stem)
{
	std::vector<Joint> joints = stem->getJoints();
	for (Joint joint : joints) {
		Path path = stem->getPath();
		BOOST_TEST(joint.getLocation().y >= 0.0f);
		BOOST_TEST(joint.getLocation().y <= 100.0f);
		BOOST_TEST(joint.getPathIndex() < path.getSize());
	}

	Stem *child = stem->getChild();
	while (child) {
		validateJoints(child);
		child = stem->getSibling();
	}
}

BOOST_AUTO_TEST_CASE(test_path_index)
{
	Plant plant;
	plant.addMaterial(Material());
	plant.addCurve(Curve(1));

	Path path = createPath();
	Stem *root = plant.createRoot();
	root->setPath(path);
	addStem(plant, root, path, 0.0f);
	addStem(plant, root, path, 0.5f);
	addStem(plant, root, path, 1.0f);
	addStem(plant, root, path, 2.0f);
	addStem(plant, root, path, 2.5f);
	addStem(plant, root, path, 3.0f);

	Wind wind;
	wind.generate(&plant);
	validateJoints(root);
}

BOOST_AUTO_TEST_SUITE_END()
