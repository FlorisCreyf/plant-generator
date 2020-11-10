#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../math/vec3.h"
#include "../math/quat.h"
#include "../math/intersection.h"
#include "../geometry.h"

const float pi = 3.14159265359f;
const float tolerance = 0.000001f;

using namespace pg;
namespace bt = boost::unit_test;

BOOST_AUTO_TEST_SUITE(vector_functions)

BOOST_AUTO_TEST_CASE(test_vec3_layout)
{
	Vec3 vector;
	BOOST_TEST(sizeof(vector) == 12);
	BOOST_TEST(offsetof(Vec3, x) == 0);
	BOOST_TEST(offsetof(Vec3, y) == 4);
	BOOST_TEST(offsetof(Vec3, z) == 8);
	BOOST_TEST(std::is_standard_layout<Vec3>::value);
}

BOOST_AUTO_TEST_CASE(test_vec4_layout)
{
	Vec4 vector;
	BOOST_TEST(sizeof(vector) == 16);
	BOOST_TEST(offsetof(Vec4, x) == 0);
	BOOST_TEST(offsetof(Vec4, y) == 4);
	BOOST_TEST(offsetof(Vec4, z) == 8);
	BOOST_TEST(offsetof(Vec4, w) == 12);
	BOOST_TEST(std::is_standard_layout<Vec4>::value);
}

BOOST_AUTO_TEST_CASE(test_vec2_magnitude)
{
	Vec2 vec(0, 2);
	BOOST_TEST(magnitude(vec) == 2);
}

BOOST_AUTO_TEST_CASE(test_rectangle_intersection, *bt::tolerance(tolerance))
{
	Ray ray;
	ray.origin = Vec3(0.0f, 2.5f, 0.5f);
	ray.direction = Vec3(0.0f, -1.0f, 0.0f);
	Vec3 a(0.5f, 0.5f, 0.0f);
	Vec3 b(0.5f, 0.5f, 1.0f);
	Vec3 d(-0.5f, 0.5f, 0.0f);
	/* The distance of the intersection is returned. */
	BOOST_TEST(intersectsRectangle(ray, a, b, d) == 2.0f);
	/* A non-zero distance is returned. */
	ray.direction.x = 0.1f;
	ray.direction = normalize(ray.direction);
	BOOST_TEST(intersectsRectangle(ray, a, b, d) > 0.0f);
	ray.origin.x = 0.6f;
	/* No intersection occurred and zero is returned. */
	BOOST_TEST(intersectsRectangle(ray, a, b, d) == 0.0f);
}

BOOST_AUTO_TEST_CASE(test_rotate_around_axis, *bt::tolerance(tolerance))
{
	/* Rotate 180 degrees around the y-axis. */
	Quat rotation1 = fromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), pi);
	Quat rotation2(0.0f, 1.0f, 0.0f, 0.0f);
	BOOST_TEST(rotation1.x == rotation2.x);
	BOOST_TEST(rotation1.y == rotation2.y);
	BOOST_TEST(rotation1.z == rotation2.z);
	BOOST_TEST(rotation1.w == rotation2.w);

	Vec3 vec(0.0f, 0.0f, 1.0f);
	vec = rotate(rotation2, vec);
	BOOST_TEST(vec.x == 0.0f);
	BOOST_TEST(vec.y == 0.0f);
	BOOST_TEST(vec.z == -1.0f);
}

BOOST_AUTO_TEST_CASE(test_clamp_direction, *bt::tolerance(tolerance))
{
	float max = std::sqrt(2.0f)/2.0f;
	Vec3 a(0.0f, std::sin(pi*0.4f), std::cos(pi*0.4f));
	Vec3 b(0.0f, 0.0f, 1.0f);
	BOOST_TEST(dot(a, b) < max);
	Vec3 rejection = normalize(a - dot(a, b)*b);
	Vec3 limited = max*rejection + max*b;
	BOOST_TEST(limited.x == 0.0f);
	BOOST_TEST(limited.y == max);
	BOOST_TEST(limited.z == max);
}

BOOST_AUTO_TEST_SUITE_END()
