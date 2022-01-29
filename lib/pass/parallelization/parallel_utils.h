/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:31:19
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-28 14:50:04
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"

namespace polly {

namespace internal {

struct Node;

typedef std::shared_ptr<Node> NodeHandle;

struct Node {
  std::string id;
  // -1 : Undefined, >=0 : concrete component id
  int component_id;
  int low_link;
  std::map<std::string, NodeHandle> outgoings;
  int indegree;

  Node() {
    component_id = -1;
    id = "";
    outgoings.clear();
    indegree = 0;
  }
  Node(std::string id) : id(id) {
    component_id = -1;
    outgoings.clear();
    indegree = 0;
  }
  Node(std::string id, std::map<std::string, NodeHandle> outgoings)
      : id(id), outgoings(outgoings) {
    component_id = -1;
    indegree = 0;
  }
  static NodeHandle create(std::string id) {
    return std::shared_ptr<Node>(new Node(id));
  }
  // bool operator==(const Node& other) { return id == other.id; }
};

// Tarjan's strongly connected components algorithm
class TarjanSCC {
 public:
  TarjanSCC() { index = 0; }

  void FindSCC();

  void StrongConnect(NodeHandle node);

  void AddNode(NodeHandle node) {
    if (!Contains(node)) nodes[node->id] = node;
  }

  void AddEdge(NodeHandle src, NodeHandle dst) {
    assert(Contains(src));
    assert(Contains(dst));
    src->outgoings.insert({dst->id, dst});
    dst->indegree += 1;
  }

  bool OnStack(NodeHandle node) {
    for (int i = 0; i < stack.size(); i++) {
      if (stack[i]->id == node->id) return true;
    }
    return false;
  }

  bool Contains(NodeHandle node) { return nodes.find(node->id) != nodes.end(); }

  int index;
  std::vector<NodeHandle> stack;

  std::map<std::string, NodeHandle> nodes;
};

class TopologicalSort {
 public:
  TopologicalSort() {}

  void AddNode(NodeHandle node) {
    if (!Contains(node)) nodes[node->id] = node;
  }
  bool Contains(NodeHandle node) { return nodes.find(node->id) != nodes.end(); }

  void AddEdge(NodeHandle src, NodeHandle dst) {
    assert(Contains(src));
    assert(Contains(dst));
    src->outgoings.insert({dst->id, dst});
    dst->indegree += 1;
  }

  std::vector<NodeHandle> Sort();

  std::map<std::string, NodeHandle> nodes;
};

}  // namespace internal

}  // namespace polly
