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
	Stem *stem = pool.allocate();
	pool.deallocate(stem);
	BOOST_TEST(stem == pool.allocate());
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
