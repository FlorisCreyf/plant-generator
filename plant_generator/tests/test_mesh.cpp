#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../mesh.h"
#include "../patterns.h"

using namespace pg;
namespace bt = boost::unit_test;

BOOST_AUTO_TEST_SUITE(mesh)

BOOST_AUTO_TEST_CASE(test_index_bounds)
{
	Plant plant;
	Stem *root = plant.createRoot();

	{
		Path path;
		Spline spline;
		spline.setDegree(1);
		spline.addControl(Vec3(0.0f, 0.0f, 0.0f));
		spline.addControl(Vec3(0.0f, 10.0f, 0.0f));
		path.setSpline(spline);
		path.setRadius(getDefaultCurve(0));
		path.setMaxRadius(1.0f);
		root->setPath(path);
	}

	{
		Stem *stem = plant.addStem(root);
		Path path;
		Spline spline;
		spline.setDegree(1);
		spline.addControl(Vec3(0.0f, 0.0f, 0.0f));
		spline.addControl(Vec3(4.0f, 0.0f, 0.0f));
		path.setSpline(spline);
		path.setRadius(getDefaultCurve(0));
		path.setMaxRadius(0.1f);
		path.setMinRadius(0.0f);
		stem->setPath(path);
		stem->setPosition(5.0f);
		stem->setSwelling(Vec2(1.1f, 1.1f));
	}

	Mesh mesh(&plant);
	mesh.generate();
	size_t zeroCount = 0;
	size_t vertexCount = mesh.getVertexCount();
	std::vector<unsigned> indices = mesh.getIndices();

	BOOST_TEST(indices.size() > 0);
	BOOST_TEST(vertexCount > 0);
	for (unsigned index : indices) {
		BOOST_TEST(index < vertexCount);
		if (index == 0)
			zeroCount++;
	}
	BOOST_TEST(zeroCount < 3);
}

BOOST_AUTO_TEST_SUITE_END()
