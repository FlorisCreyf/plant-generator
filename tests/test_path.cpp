#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../plant_generator/path.h"

using namespace pg;
namespace bt = boost::unit_test;

BOOST_AUTO_TEST_SUITE(path)

Spline createCubicSpline()
{
	Spline spline;
	spline.setDegree(3);
	spline.addControl(Vec3(0.0f, 0.0f, 0.0f));
	for (int i = 0; i < 2; i++) {
		spline.addControl(Vec3(0.0f, i+0.5f, 0.0f));
		spline.addControl(Vec3(0.0f, i+0.5f, 0.0f));
		spline.addControl(Vec3(0.0f, i+1.0f, 0.0f));
	}
	return spline;
}

BOOST_AUTO_TEST_CASE(test_get_index)
{
	Path path;
	path.setSpline(createCubicSpline());
	BOOST_TEST(path.toPathIndex(0) == 0);
	BOOST_TEST(path.toPathIndex(1) == 0);
	BOOST_TEST(path.toPathIndex(2) == 0);
	BOOST_TEST(path.toPathIndex(3) == 1);
}

BOOST_AUTO_TEST_SUITE_END()
