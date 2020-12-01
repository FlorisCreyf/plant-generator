#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../plant_generator/plant.h"
#include "../plant_generator/stem_pool.h"

using namespace pg;
namespace bt = boost::unit_test;

BOOST_AUTO_TEST_SUITE(path)

BOOST_AUTO_TEST_CASE(test_allocate)
{
	StemPool pool;
	const int poolCount = 3;
	Stem *stems[poolCount][PG_POOL_SIZE];

	for (long j = 1; j <= poolCount; j++) {
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

	for (long j = poolCount-1; j >= 0; j--) {
		for (size_t i = 0; i < PG_POOL_SIZE; i++) {
			size_t remaining = pool.deallocate(stems[j][i]);
			BOOST_TEST(remaining == i + 1);
		}
	}

	BOOST_TEST(pool.getPoolCount() == poolCount);
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

BOOST_AUTO_TEST_CASE(test_last_stem_is_first)
{
	Plant plant;
	Stem *root = plant.createRoot();
	plant.addStem(root);
	Stem *stem = plant.addStem(root);
	BOOST_TEST(root->getChild() == stem);
}

BOOST_AUTO_TEST_CASE(test_extract_add_remove_reinsert)
{
	Plant plant;

	Stem *root1 = plant.addStem(nullptr);
	Stem *stem01 = plant.addStem(root1);
	Stem *stem01a = plant.addStem(stem01);
	plant.addStem(stem01);
	Stem *stem02 = plant.addStem(root1);
	Stem *stem02a = plant.addStem(stem02);
	plant.addStem(stem02);
	std::vector<Stem> extractions;
	plant.extractStems(root1, extractions);

	BOOST_TEST(extractions.size() == 7);

	BOOST_TEST(extractions[0].getSibling() == stem02a);
	BOOST_TEST(extractions[0].getParent() == stem02);

	BOOST_TEST(extractions[1].getSibling() == nullptr);
	BOOST_TEST(extractions[1].getParent() == stem02);

	BOOST_TEST(extractions[2].getSibling() == stem01);
	BOOST_TEST(extractions[2].getParent() == root1);

	BOOST_TEST(extractions[3].getSibling() == stem01a);
	BOOST_TEST(extractions[3].getParent() == stem01);

	BOOST_TEST(extractions[4].getSibling() == nullptr);
	BOOST_TEST(extractions[4].getParent() == stem01);

	BOOST_TEST(extractions[5].getSibling() == nullptr);
	BOOST_TEST(extractions[5].getParent() == root1);

	BOOST_TEST(extractions[6].getSibling() == nullptr);
	BOOST_TEST(extractions[6].getParent() == nullptr);

	Stem *root2 = plant.addStem(nullptr);
	Stem *stem1 = plant.addStem(root2);
	Stem *stem2 = plant.addStem(root2);
	Stem *stem3 = plant.addStem(root2);
	plant.deleteStem(stem3);
	plant.deleteStem(stem2);
	plant.deleteStem(stem1);
	plant.deleteStem(root2);

	plant.reinsertStems(extractions);
	BOOST_TEST(plant.getRoot() == root1);
	BOOST_TEST(plant.getRoot()->getChild() == stem02);
	BOOST_TEST(plant.getRoot()->getChild()->getSibling() == stem01);
	extractions.clear();
	plant.extractStems(root1, extractions);

	BOOST_TEST(root2 == plant.addStem(nullptr));
	BOOST_TEST(stem1 == plant.addStem(root2));
	BOOST_TEST(stem2 == plant.addStem(root2));
	BOOST_TEST(stem3 == plant.addStem(root2));
}

BOOST_AUTO_TEST_CASE(test_add_and_extract)
{
	Plant plant;
	Stem *root = plant.createRoot();
	Stem *branch1 = plant.addStem(root);
	Stem *twigs[10];
	for (int i = 0; i < 10; i++)
		twigs[i] = plant.addStem(branch1);
	Stem *branch2 = plant.addStem(root);
	plant.extractStem(branch2);
	BOOST_TEST(branch1->getChild() == twigs[9]);
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
	BOOST_TEST(root->getChild() == stem3);

	std::vector<Stem> stems;
	plant.extractStems(stem1, stems);
	plant.extractStems(stem3, stems);

	BOOST_TEST(stem2->getSibling() == nullptr);
	BOOST_TEST(root->getChild() == stem2);

	plant.reinsertStems(stems);

	BOOST_TEST(root->getChild() == stem3);
	BOOST_TEST(stem3->getSibling() == stem2);
	BOOST_TEST(stem2->getSibling() == stem1);
	BOOST_TEST(stem1->getSibling() == nullptr);
}

BOOST_AUTO_TEST_SUITE_END()
