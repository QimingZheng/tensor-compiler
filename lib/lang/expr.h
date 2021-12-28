
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

class Constant : public Expr {
 public:
  std::string name;
  Constant(const std::string name);
};

}  // namespace polly