#include "scc.h"

namespace polly {

namespace internal {

void TarjanSCC::FindSCC() {
  for (auto &it : nodes) {
    auto &node = it.second;
    if (node->component_id < 0) {
      std::cout << node->id << std::endl;
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

}  // namespace internal

}  // namespace polly
