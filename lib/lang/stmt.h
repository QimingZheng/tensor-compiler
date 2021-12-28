#pragma once

#include "common.h"

namespace polly {

class Expr;

class Stmt {
 public:
  mutable IRHandle handle_;
  IRHandle GetIRHandle() const { return handle_; }
};

/// All Assignments will be registered into the ast tree.
class Assignment : public Stmt {
 public:
  Assignment(const Expr lhs, const Expr &rhs);
};

class Print : public Stmt {
 public:
  Print(const Expr print);
};

}  // namespace polly