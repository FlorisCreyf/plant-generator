#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../plant_generator/plant.h"
#include "../plant_generator/stem_pool.h"
#include "../editor/commands/generate.h"
#include <vector>

using namespace pg;
using std::vector;
namespace bt = boost::unit_test;

BOOST_AUTO_TEST_SUITE(commands)

void addAllocations(Stem *stem, vector<Stem *> &allocations)
{
	allocations.push_back(stem);
	Stem *child = stem->getChild();
	while (child) {
		addAllocations(child, allocations);
		child = child->getSibling();
	}
}

void compareAllocations(Stem *stem, vector<Stem *> &allocations, size_t &index)
{
	BOOST_TEST(stem == allocations[index]);
	Stem *child = stem->getChild();
	while (child) {
		++index;
		compareAllocations(child, allocations, index);
		child = child->getSibling();
	}
}

void compareAllocations(Stem *stem, vector<Stem *> &allocations)
{
	size_t index = 0;
	compareAllocations(stem, allocations, index);
}

void initializeParameterTree(ParameterTree &ptree)
{
	ptree.createRoot();
	StemData stemData;
	stemData.density = 1.0f;
	stemData.start = 0.5f;
	stemData.length = 50.0f;
	stemData.leaf.density = 0.5f;
	ParameterNode *pnode;
	pnode = ptree.addChild("");
	pnode->setData(stemData);
	pnode = ptree.addChild("1");
	pnode->setData(stemData);
	pnode = ptree.addSibling("1");
	pnode->setData(stemData);
}

BOOST_AUTO_TEST_CASE(test_remove)
{
	Plant plant;
	plant.setDefault();
	ParameterTree ptree;
	initializeParameterTree(ptree);
	PatternGenerator generator(&plant);
	generator.setParameterTree(ptree);
	generator.grow();

	vector<Stem *> initialAllocations;
	addAllocations(plant.getRoot(), initialAllocations);

	Selection selection(&plant);
	selection.addStem(plant.getRoot());
	RemoveStem remove(&selection);
	remove.execute();
	remove.undo();
	compareAllocations(plant.getRoot(), initialAllocations);
}

BOOST_AUTO_TEST_CASE(test_generate)
{
	Plant plant;
	plant.setDefault();
	ParameterTree ptree;
	initializeParameterTree(ptree);

	PatternGenerator generator(&plant);
	generator.setParameterTree(ptree);
	generator.grow();

	vector<Stem *> initialAllocations;
	addAllocations(plant.getRoot(), initialAllocations);

	Selection selection(&plant);
	selection.addStem(plant.getRoot());

	Generate generate(&selection, &generator);
	generate.execute();
	generate.execute();
	generate.undo();
	compareAllocations(plant.getRoot(), initialAllocations);
	generate.redo();
	generate.undo();
	compareAllocations(plant.getRoot(), initialAllocations);
}

BOOST_AUTO_TEST_CASE(test_add_remove)
{
	Plant plant;
	plant.setDefault();

	Stem *root = plant.createRoot();
	for (int i = 0; i < PG_POOL_SIZE; i++)
		plant.addStem(root);

	Selection selection(&plant);
	selection.addStem(root);
	selection.selectChildren();

	RemoveStem remove(&selection);
	remove.execute();
	remove.undo();
}

BOOST_AUTO_TEST_SUITE_END()
