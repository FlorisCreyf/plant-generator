#define BOOST_TEST_MODULE boost_test_math

#include <boost/test/included/unit_test.hpp>
#include "../plant_generator/math/math.h"
#include <type_traits>

BOOST_AUTO_TEST_CASE(test_vec3_layout)
{
	pg::Vec3 vector;
	BOOST_TEST(sizeof(vector) == 12);
	BOOST_TEST(offsetof(pg::Vec3, x) == 0);
	BOOST_TEST(offsetof(pg::Vec3, y) == 4);
	BOOST_TEST(offsetof(pg::Vec3, z) == 8);
	BOOST_TEST(std::is_pod<pg::Vec3>::value);
}

BOOST_AUTO_TEST_CASE(test_vec4_layout)
{
	pg::Vec4 vector;
	BOOST_TEST(sizeof(vector) == 16);
	BOOST_TEST(offsetof(pg::Vec4, x) == 0);
	BOOST_TEST(offsetof(pg::Vec4, y) == 4);
	BOOST_TEST(offsetof(pg::Vec4, z) == 8);
	BOOST_TEST(offsetof(pg::Vec4, w) == 12);
	BOOST_TEST(std::is_pod<pg::Vec4>::value);
}

BOOST_AUTO_TEST_CASE(test_vec2_magnitude)
{
	pg::Vec2 vec = {0, 2};
	BOOST_TEST(magnitude(vec) == 2);
}
