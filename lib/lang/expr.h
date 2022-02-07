/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:32:21
 * @Last Modified by:   Qiming Zheng
 * @Last Modified time: 2022-01-18 20:32:21
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "ir/ir.h"
#include "stmt.h"

namespace polly {

class Expr {
 public:
  Expr() = default;
  mutable IRHandle handle_;
  explicit Expr(IRHandle handle) : handle_(handle) {}
  IRHandle GetIRHandle() const { return handle_; }

  Expr(int x) : handle_(IntNode::make(x)) {}
};

Expr operator+(const Expr &a, const Expr &b);
Expr operator-(const Expr &a, const Expr &b);
Expr operator*(const Expr &a, const Expr &b);
Expr operator/(const Expr &a, const Expr &b);

/// Looping itearator
class Variable : public Expr {
 public:
  Variable() = delete;
  /// For (i = min; i < max; i+= increment)
  Variable(const Expr &min, const Expr &max, const Expr &increment);
  ~Variable();
  Variable(const Variable &other) {
    // Don't register when copy
  }
  Variable &operator=(const Variable &other) {
    // Don't register when copy
  }

  IRNodeKey id;
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
  IRNodeKey id;
  Tensor(std::vector<int64_t> shape);

  Access operator()(const std::vector<Expr> &indices) const {
    return Access(*this, indices);
  }

  template <typename... Index>
  const Access operator()(const Index &...indices) const {
    return static_cast<const Tensor *>(this)->operator()({indices...});
  }
};

/// TODO: Should not be exposed to users.
class Constant : public Expr {
 public:
  std::string name;
  Constant(const std::string name);
};

class Min : public Expr {
 public:
  Min(const Expr &a, const Expr &b);
};

class Max : public Expr {
 public:
  Max(const Expr &a, const Expr &b);
};

}  // namespace polly