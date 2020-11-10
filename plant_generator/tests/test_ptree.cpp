#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../parameter_tree.h"
#include <algorithm>

using namespace pg;
namespace bt = boost::unit_test;

BOOST_AUTO_TEST_SUITE(parameter_tree)

BOOST_AUTO_TEST_CASE(test_get_names)
{
	ParameterTree tree;
	tree.createRoot();
	tree.addChild("");
	tree.addSibling("1");
	tree.addChild("2");
	tree.addSibling("2.1");
	tree.addSibling("2.1");
	auto names = tree.getNames();
	auto first = names.begin();
	auto last = names.end();
	BOOST_TEST(names.size() == 5);
	BOOST_TEST((std::find(first, last, "1") != last));
	BOOST_TEST((std::find(first, last, "2") != last));
	BOOST_TEST((std::find(first, last, "2.1") != last));
	BOOST_TEST((std::find(first, last, "2.2") != last));
	BOOST_TEST((std::find(first, last, "2.3") != last));

	tree.remove("2.1");
	names = tree.getNames();
	first = names.begin();
	last = names.end();
	BOOST_TEST(names.size() == 4);
	BOOST_TEST((std::find(first, last, "1") != last));
	BOOST_TEST((std::find(first, last, "2") != last));
	BOOST_TEST((std::find(first, last, "2.1") != last));
	BOOST_TEST((std::find(first, last, "2.2") != last));

	tree.remove("2");
	names = tree.getNames();
	BOOST_TEST(names.size() == 1);
	BOOST_TEST(names[0] == "1");

	tree.reset();
	names = tree.getNames();
	BOOST_TEST(names.empty());
}

BOOST_AUTO_TEST_CASE(test_assignment)
{
	ParameterTree tree;
	tree.createRoot();
	tree.addChild("");
	tree.addChild("1");
	tree.addChild("1.1");
	tree.addSibling("1");
	tree.addChild("2");
	ParameterTree treeCopy;
	treeCopy.createRoot();
	treeCopy.addChild("1");
	treeCopy.addSibling("1.1");
	treeCopy = tree;

	BOOST_TEST(tree.getRoot() != treeCopy.getRoot());

	auto names = treeCopy.getNames();
	auto first = names.begin();
	auto last = names.end();
	BOOST_TEST(names.size() == 5);
	BOOST_TEST((std::find(first, last, "1") != last));
	BOOST_TEST((std::find(first, last, "1.1") != last));
	BOOST_TEST((std::find(first, last, "1.1.1") != last));
	BOOST_TEST((std::find(first, last, "2") != last));
	BOOST_TEST((std::find(first, last, "2.1") != last));
}

BOOST_AUTO_TEST_CASE(test_nonexistent)
{
	ParameterTree tree;
	tree.createRoot();
	BOOST_TEST(tree.addChild("2") == nullptr);
	BOOST_TEST(tree.addSibling("1.1") == nullptr);
	BOOST_TEST(!tree.remove("3.2"));
}

BOOST_AUTO_TEST_CASE(test_structure)
{
	ParameterTree tree;
	ParameterRoot *root = tree.createRoot();
	ParameterNode *node1 = tree.addChild("");
	ParameterNode *node2 = tree.addSibling("1");
	ParameterNode *node1_1 = tree.addChild("1");
	ParameterNode *node1_2 = tree.addSibling("1.1");

	BOOST_TEST(node2->getPrevSibling() == node1);
	BOOST_TEST(node2->getNextSibling() == nullptr);

	BOOST_TEST(node1_1->getPrevSibling() == nullptr);
	BOOST_TEST(node1_1->getNextSibling() == node1_2);
	BOOST_TEST(node1_1->getParent() == node1);
	BOOST_TEST(node1_1->getChild() == nullptr);

	BOOST_TEST(node1_2->getPrevSibling() == node1_1);
	BOOST_TEST(node1_2->getNextSibling() == nullptr);
	BOOST_TEST(node1_2->getParent() == node1);
	BOOST_TEST(node1_2->getChild() == nullptr);
}

BOOST_AUTO_TEST_CASE(test_remove_second_sibling)
{
	ParameterTree tree;
	tree.createRoot();
	tree.addChild("");
	tree.addChild("1");
	tree.addSibling("1.1");
	BOOST_TEST(tree.remove("1.2"));
	ParameterNode *node = tree.getRoot()->getNode();
	BOOST_TEST(node);
	BOOST_TEST(node->getChild());
	BOOST_TEST(node->getChild()->getParent() == node);
	BOOST_TEST(!node->getChild()->getChild());
	BOOST_TEST(!node->getChild()->getNextSibling());
	BOOST_TEST(!node->getChild()->getPrevSibling());
}

BOOST_AUTO_TEST_CASE(test_remove_first_sibling)
{
	ParameterTree tree;
	tree.createRoot();
	tree.addChild("");
	tree.addChild("1");
	tree.addSibling("1.1");
	BOOST_TEST(tree.remove("1.1"));
	ParameterNode *node = tree.getRoot()->getNode();
	BOOST_TEST(node);
	BOOST_TEST(node->getChild());
	BOOST_TEST(node->getChild()->getParent() == node);
	BOOST_TEST(!node->getChild()->getChild());
	BOOST_TEST(!node->getChild()->getNextSibling());
	BOOST_TEST(!node->getChild()->getPrevSibling());
}

BOOST_AUTO_TEST_SUITE_END()
