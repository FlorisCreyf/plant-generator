#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../plant.h"
#include "../stem_pool.h"

using namespace pg;
namespace bt = boost::unit_test;

#define NUM_POOLS 3

BOOST_AUTO_TEST_SUITE(path)

BOOST_AUTO_TEST_CASE(test_allocate)
{
	StemPool pool;
	Stem *stems[NUM_POOLS][PG_POOL_SIZE];

	for (long j = 1; j <= NUM_POOLS; j++) {
		for (size_t i = 0; i < PG_POOL_SIZE; i++) {
			Stem *stem = pool.allocate();
			BOOST_TEST(stem != nullptr);
			long id = pool.getPoolID(stem);
			BOOST_TEST(id == j);
			size_t remaining = PG_POOL_SIZE - i - 1;
			BOOST_TEST(pool.getPoolCount() == j);
			BOOST_TEST(pool.getRemaining(id) == remaining);
			stems[j-1][i] = stem;
		}
	}

	for (long j = NUM_POOLS-1; j >= 0; j--) {
		for (size_t i = 0; i < PG_POOL_SIZE; i++) {
			size_t remaining = pool.deallocate(stems[j][i]);
			BOOST_TEST(remaining == i + 1);
		}
	}

	BOOST_TEST(pool.getPoolCount() == NUM_POOLS);
	Stem *stem = pool.allocate();
	BOOST_TEST(pool.getPoolID(stem) == 1);
}

BOOST_AUTO_TEST_CASE(test_same_address)
{
	StemPool pool;
	Stem *stem1 = pool.allocate();
	Stem *stem2 = pool.allocate();
	Stem *stem3 = pool.allocate();
	Stem *stem4 = pool.allocate();
	pool.deallocate(stem4);
	pool.deallocate(stem3);
	pool.deallocate(stem2);
	pool.deallocate(stem1);
	BOOST_TEST(stem1 == pool.allocate());
}

BOOST_AUTO_TEST_CASE(test_extract_and_allocate)
{
	Plant plant;
	Stem *root1 = plant.addStem(nullptr);
	for (int i = 0; i < 2; i++)
		plant.addStem(root1);

	std::vector<Plant::Extraction> extractions;
	plant.extractStems(root1, extractions);

	Stem *root2 = plant.addStem(nullptr);
	Stem *stem1 = plant.addStem(root2);
	Stem *stem2 = plant.addStem(root2);
	Stem *stem3 = plant.addStem(root2);
	plant.deleteStem(stem3);
	plant.deleteStem(stem2);
	plant.deleteStem(stem1);
	plant.deleteStem(root2);

	/* This will change the order of siblings. */
	plant.reinsertStems(extractions);
	extractions.clear();
	plant.extractStems(root1, extractions);

	BOOST_TEST(root2 == plant.addStem(nullptr));
	BOOST_TEST(stem1 != plant.addStem(root2));
	BOOST_TEST(stem2 != plant.addStem(root2));
	BOOST_TEST(stem3 == plant.addStem(root2));
}

BOOST_AUTO_TEST_CASE(test_undo_redo)
{
	Plant plant;
	Stem *root = plant.createRoot();
	Stem *branch1 = plant.addStem(root);
	Stem *stems[10];
	for (int i = 0; i < 10; i++)
		stems[i] = plant.addStem(branch1);
	Stem *branch2 = plant.addStem(root);
	plant.extractStem(branch2);
	BOOST_TEST(branch1->getChild() == stems[9]);
	BOOST_TEST(branch1->getSibling() == nullptr);
	BOOST_TEST(branch1->getParent() == root);
	BOOST_TEST(root->getChild() == branch1);
}

BOOST_AUTO_TEST_CASE(test_extract)
{
	Plant plant;
	Stem *root = plant.createRoot();
	Stem *stem1 = plant.addStem(root);
	Stem *stem2 = plant.addStem(root);
	Stem *stem3 = plant.addStem(root);
	std::vector<Plant::Extraction> stems;
	plant.extractStems(stem1, stems);
	plant.extractStems(stem3, stems);
	BOOST_TEST(stem2->getSibling() == nullptr);
	BOOST_TEST(root->getChild() == stem2);
	plant.reinsertStems(stems);
	BOOST_TEST(root->getChild() == stem3);
	BOOST_TEST(stem3->getSibling() == stem1);
	BOOST_TEST(stem1->getSibling() == stem2);
	BOOST_TEST(stem2->getSibling() == nullptr);
}

BOOST_AUTO_TEST_SUITE_END()
