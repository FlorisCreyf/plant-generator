#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../plant_generator/volume.h"

using namespace pg;
namespace bt = boost::unit_test;

const float tolerance = 0.000001f;

BOOST_AUTO_TEST_SUITE(octree)

BOOST_AUTO_TEST_CASE(test_get_node, *bt::tolerance(tolerance))
{
	Volume volume(1.0f, 3);
	Vec3 point(0.76f-0.5f, 0.126f-0.5f, 0.26f);
	Volume::Node *node1 = volume.addNode(point);
	Volume::Node *node2 = volume.getNode(point);
	BOOST_TEST(node1 == node2);
	Vec3 center = node1->getCenter();
	BOOST_TEST(center.x == 0.8125f-0.5f);
	BOOST_TEST(center.y == 0.1875f-0.5f);
	BOOST_TEST(center.z == 0.3125f);
}

BOOST_AUTO_TEST_CASE(test_add_line)
{
	/* This test will fail if the ray direction is diagonal to the volume
	due to floating point precision errors. */
	Volume volume(1.0f, 3);
	Vec3 a(-0.5f, -0.5f, 0.1f);
	Vec3 b(0.99f-0.5f, 0.01f-0.5f, 0.01f);
	Vec3 c(0.51f, 0.50f, 0.1f);
	float weight = 0.6f;

	volume.addLine(a, b, weight, 0.001f);
	Volume::Node *node1 = volume.getNode(a);
	Volume::Node *node2 = volume.getNode(b);
	BOOST_TEST(node1->getDepth() == 3);
	BOOST_TEST(node2->getDepth() == 3);
	BOOST_TEST(node1->getDensity() == weight);
	BOOST_TEST(node2->getDensity() == weight);

	volume.addLine(a, c, weight, 0.001f);
	Volume::Node *node3 = volume.getNode(c);
	BOOST_TEST(node3->getDepth() == 3);
	BOOST_TEST(node3->getDensity() == weight);
}

BOOST_AUTO_TEST_SUITE_END()
