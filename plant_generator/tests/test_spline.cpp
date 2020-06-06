#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../spline.h"

#define PI 3.14159265359f
#define TOLERANCE 0.000001f

using namespace pg;
namespace bt = boost::unit_test;

BOOST_AUTO_TEST_SUITE(spline)

std::vector<Vec3> getRandomControls()
{
	std::vector<Vec3> controls;
	/* Curve 1 */
	controls.push_back(Vec3(0.0f, 0.0f, 0.0f));
	controls.push_back(Vec3(0.0f, 2.0f, 0.0f));
	controls.push_back(Vec3(1.0f, 0.0f, 0.0f));
	controls.push_back(Vec3(1.0f, 1.0f, 0.0f));
	/* Curve 2 */
	controls.push_back(Vec3(1.0f, 2.0f, 0.0f));
	controls.push_back(Vec3(1.0f, 0.0f, 0.0f));
	controls.push_back(Vec3(1.0f, 0.0f, 7.0f));
	return controls;
}

BOOST_AUTO_TEST_CASE(test_to_linear, *bt::tolerance(TOLERANCE))
{
	Spline spline;
	std::vector<Vec3> controls = getRandomControls();
	spline.setDegree(3);
	spline.setControls(controls);
	spline.adjust(1);
	std::vector<Vec3> result = spline.getControls();
	BOOST_TEST(result[0] == controls[0]);
	BOOST_TEST(result[1] == controls[3]);
	BOOST_TEST(result[2] == controls[6]);
}

BOOST_AUTO_TEST_CASE(test_move_cubic, *bt::tolerance(TOLERANCE))
{
	Spline spline;
	spline.setDegree(3);
	std::vector<Vec3> controls = getRandomControls();
	spline.setControls(controls);
	spline.move(3, Vec3(7.0f, 7.0f, 7.0f), false);
	std::vector<Vec3> result = spline.getControls();
	BOOST_TEST(result[3].x == 7.0f);
	BOOST_TEST(result[3].y == 7.0f);
	BOOST_TEST(result[3].z == 7.0f);

	BOOST_TEST(result[4].x == controls[4].x - controls[3].x + 7.0f);
	BOOST_TEST(result[4].y == controls[4].y - controls[3].y + 7.0f);
	BOOST_TEST(result[4].z == controls[4].z - controls[3].z + 7.0f);

	BOOST_TEST(result[2].x == controls[2].x - controls[3].x + 7.0f);
	BOOST_TEST(result[2].y == controls[2].y - controls[3].y + 7.0f);
	BOOST_TEST(result[2].z == controls[2].z - controls[3].z + 7.0f);
}

BOOST_AUTO_TEST_CASE(test_parallelize_cubic, *bt::tolerance(TOLERANCE))
{
	Spline spline;
	std::vector<Vec3> controls = getRandomControls();
	controls[2] = Vec3(1.0f, 0.0f, 0.0f);
	controls[3] = Vec3(1.0f, 1.0f, 0.0f);
	controls[4] = Vec3(1.0f, 2.0f, 0.0f);
	
	spline.setDegree(3);
	spline.setControls(controls);
	spline.parallelize(2);

	Vec3 point = spline.getControls()[4];
	BOOST_TEST(point.x == controls[4].x);
	BOOST_TEST(point.y == controls[4].y);
	BOOST_TEST(point.z == controls[4].z);
	
	controls[4] = point;
	controls[2].y = -1.0f;
	spline.setControls(controls);
	spline.parallelize(4);
	
	point = spline.getControls()[2];
	BOOST_TEST(point.x == controls[2].x);
	BOOST_TEST(point.y == controls[2].y);
	BOOST_TEST(point.z == controls[2].z);
}

BOOST_AUTO_TEST_CASE(test_parallelize_linear)
{
	Spline spline;
	std::vector<Vec3> controls;
	controls.push_back(Vec3(0.0f, 0.0f, 0.0f));
	controls.push_back(Vec3(0.0f, 2.0f, 0.0f));
	spline.setDegree(1);
	spline.setControls(controls);
	spline.parallelize(0);
	spline.parallelize(1);
	BOOST_TEST(controls[0] == spline.getControls()[0]);
	BOOST_TEST(controls[1] == spline.getControls()[1]);
}

BOOST_AUTO_TEST_SUITE_END()
