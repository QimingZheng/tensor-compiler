#include "parallel_utils.h"

namespace polly {

namespace internal {

void TarjanSCC::FindSCC() {
  for (auto &it : nodes) {
    auto &node = it.second;
    if (node->component_id < 0) {
      StrongConnect(node);
    }
  }
}

void TarjanSCC::StrongConnect(NodeHandle node) {
  //   stack.clear();
  node->component_id = index;
  node->low_link = index;
  index++;
  stack.push_back(node);

  for (auto &it : node->outgoings) {
    auto &v = it.second;
    if (v->component_id < 0) {
      StrongConnect(v);
      node->low_link = std::min(node->low_link, v->low_link);
    } else if (OnStack(v)) {
      node->low_link = std::min(node->low_link, v->component_id);
    }
  }

  if (node->component_id == node->low_link) {
    while (stack.back()->id != node->id) {
      stack.back()->component_id = node->component_id;
      stack.pop_back();
    }
    stack.pop_back();
  }
}

std::vector<NodeHandle> TopologicalSort::Sort() {
  std::vector<NodeHandle> ret;
  std::deque<NodeHandle> q;
  for (auto it : nodes) {
    if (it.second->indegree == 0) {
      q.push_back(it.second);
    }
  }
  while (!q.empty()) {
    auto cur = q.front();
    ret.push_back(cur);
    q.pop_front();
    for (auto &it : cur->outgoings) {
      it.second->indegree -= 1;
      if (it.second->indegree == 0) {
        q.push_back(it.second);
      }
    }
  }
  return ret;
}

}  // namespace internal

}  // namespace polly
