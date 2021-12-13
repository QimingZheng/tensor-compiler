#pragma once

#include "common.h"

namespace polly {

class Expr;

class Stmt {
 public:
  mutable IRNode *stmt_node_;
  IRNode *GetIRNode() const { return stmt_node_; }
};

/// All Assignments will be registered into the ast tree.
class Assignment : public Stmt {
 public:
  Assignment(const Expr lhs, const Expr &rhs);
};

}  // namespace polly