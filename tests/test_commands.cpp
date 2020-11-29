#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../plant_generator/plant.h"
#include "../plant_generator/stem_pool.h"
#include "../editor/commands/generate.h"

using namespace pg;
namespace bt = boost::unit_test;

BOOST_AUTO_TEST_SUITE(path)

BOOST_AUTO_TEST_CASE(test_generate)
{
	Plant plant;
	plant.setDefault();

	ParameterTree ptree;
	ptree.createRoot();
	ParameterNode *node = ptree.addChild("");
	StemData stemData;
	stemData.density = 1.0f;
	stemData.start = 0.5f;
	stemData.leaf.density = 0.5f;
	node->setData(stemData);

	PseudoGenerator generator(&plant);
	generator.setParameterTree(ptree);
	generator.grow();

	BOOST_TEST(plant.getRoot());

	Selection selection(&plant);
	selection.addStem(plant.getRoot());

	Generate generate(&selection);
	generate.execute();
}

BOOST_AUTO_TEST_SUITE_END()
