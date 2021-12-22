
#pragma once

#include "common.h"
#include "ir/ir.h"
#include "stmt.h"

namespace polly {

class Expr {
 public:
  Expr() = default;
  mutable IRNode *expr_node_;
  IRNode *GetIRNode() const { return expr_node_; }

  Expr(int x) : expr_node_(new IntNode(x)) {}
};

class Add : public Expr {
 public:
  Add() = delete;
  Add(const Expr &lhs, const Expr &rhs) {
    expr_node_ = new AddNode(lhs.GetIRNode(), rhs.GetIRNode());
  }
};

Add operator+(const Expr &a, const Expr &b);

class Mul : public Expr {
 public:
  Mul() = delete;
  Mul(const Expr &lhs, const Expr &rhs) {
    expr_node_ = new MulNode(lhs.GetIRNode(), rhs.GetIRNode());
  }
};

Mul operator*(const Expr &a, const Expr &b);

/// Looping itearator
class Variable : public Expr {
 public:
  Variable() = delete;
  /// For (i = min; i < max; i+= increament)
  Variable(const std::string &name, const Expr &min, const Expr &max,
           const Expr &increament);
  ~Variable();
  Variable(const Variable &other) {
    // Don't register when copy
  }
  Variable &operator=(const Variable &other) {
    // Don't register when copy
  }

  std::string name;
};

class Access : public Expr {
 public:
  Access(const Expr tensor, const std::vector<Expr> &indices);

  Assignment operator=(const Expr &rhs) const { return Assignment(*this, rhs); }
};

///
class Tensor : public Expr {
 public:
  std::vector<int64_t> shape;
  std::string name;
  Tensor(const std::string &name, std::vector<int64_t> shape);

  Access operator()(const std::vector<Expr> &indices) const {
    return Access(*this, indices);
  }

  template <typename... Index>
  const Access operator()(const Index &...indices) const {
    return static_cast<const Tensor *>(this)->operator()({indices...});
  }
};

}  // namespace polly