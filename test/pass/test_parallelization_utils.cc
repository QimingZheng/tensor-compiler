#include "gtest/gtest.h"

#include "pass/parallelization/parallel_utils.h"

using namespace polly::internal;

TEST(STRONGLY_CONNECTED_COMPONENT, SCC) {
  TarjanSCC scc;
  NodeHandle n1 = Node::create("1"), n2 = Node::create("2"),
             n3 = Node::create("3"), n4 = Node::create("4"),
             n5 = Node::create("5"), n6 = Node::create("6"),
             n7 = Node::create("7"), n8 = Node::create("8");

  scc.AddNode(n1);
  scc.AddNode(n2);
  scc.AddNode(n3);
  scc.AddNode(n4);
  scc.AddNode(n5);
  scc.AddNode(n6);
  scc.AddNode(n7);
  scc.AddNode(n8);

  scc.AddEdge(n1, n2);
  scc.AddEdge(n2, n3);
  scc.AddEdge(n3, n1);
  scc.AddEdge(n4, n2);
  scc.AddEdge(n4, n3);
  scc.AddEdge(n4, n5);
  scc.AddEdge(n5, n4);
  scc.AddEdge(n5, n6);
  scc.AddEdge(n6, n3);
  scc.AddEdge(n7, n6);
  scc.AddEdge(n6, n7);
  scc.AddEdge(n8, n8);
  scc.AddEdge(n8, n5);
  scc.AddEdge(n8, n7);

  scc.FindSCC();
  EXPECT_EQ(n1->component_id, n2->component_id);
  EXPECT_EQ(n2->component_id, n3->component_id);
  EXPECT_EQ(n4->component_id, n5->component_id);
  EXPECT_EQ(n7->component_id, n6->component_id);
  EXPECT_EQ(n8->component_id != n7->component_id, true);
}