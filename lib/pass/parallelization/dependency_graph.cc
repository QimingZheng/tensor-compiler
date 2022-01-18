#include "dependency_graph.h"

namespace polly {

DependencyGraph::DependencyGraph(PolyhedralModel model) {
  for (int i = 0; i < model.statements_.size(); i++) {
    nodes_.push_back(model.statements_[i]);
  }

  for (int i = 0; i < model.statements_.size(); i++) {
    statements_[model.statements_[i].statementName] = model.statements_[i];
  }

  for (int i = 0; i < model.statements_.size(); i++) {
    Statement s = model.statements_[i];
    for (int j = 0; j < s.accesses_.size(); j++) {
      ArrayAccess a = s.accesses_[j];
      if (a.access.type_ == ArrayDomain::AccessType::READ) {
        reads[a.access.arrayName_].push_back(a);
      } else {
        writes[a.access.arrayName_].push_back(a);
      }
    }
  }

  for (auto array : arrayKeys_) {
    // read & write
    reads[array];
    writes[array];
    // write & write
    writes[array];
  }
}
}  // namespace polly