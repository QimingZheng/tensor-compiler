#pragma once

#include "common.h"

namespace polly {

typedef std::string VarKey;
typedef std::string StatementKey;
typedef std::string ArrayKey;

/// QuasiAffineExpr represents formulas in the form of:
/// (\sum_{i = 1}^{n} c_i x_i + c_0) / d
/// where c_i is constant integer coefficients
/// x_i is integer variable
class QuasiAffineExpr {
 public:
  QuasiAffineExpr() {}
  QuasiAffineExpr(std::map<VarKey, int> coeffs, int constant, int divisor)
      : coeffs(coeffs), constant(constant), divisor(divisor) {}
  void clear() {
    coeffs.clear();
    constant = 0;
    divisor = 1;
  }
  // std::vector<VarKey> varNames;
  // std::vector<int> coeffs;
  std::map<VarKey, int> coeffs;
  int constant;
  int divisor;
};

class Iteration {
 public:
  Iteration() {}
  Iteration(VarKey iter_name, QuasiAffineExpr lower, QuasiAffineExpr upper)
      : iterName_(iter_name), lowerBound_(lower), upperBound_(upper) {
    ///
  }
  VarKey iterName_;
  QuasiAffineExpr lowerBound_;
  QuasiAffineExpr upperBound_;
  // increment is always +1.
};

class IterDomain {
 public:
  IterDomain() {}
  IterDomain(std::vector<Iteration> iterations) : iterations_(iterations) {}
  std::vector<Iteration> iterations_;
};

class ProgDomain {
 public:
  ProgDomain() {}
  ProgDomain(std::vector<int> program_context)
      : progContext_(program_context) {}

  std::vector<int> progContext_;
};

class ArrayDomain {
 public:
  enum AccessType {
    READ,
    WRITE,
  };
  ArrayDomain() {}
  ArrayDomain(AccessType type, ArrayKey arrayName,
              std::vector<QuasiAffineExpr> indices)
      : type_(type), arrayName_(arrayName), indices_(indices) {}
  AccessType type_;
  ArrayKey arrayName_;
  std::vector<QuasiAffineExpr> indices_;
};

/// An array access instance is an array access from a certain statement.
class ArrayAccess {
 public:
  ArrayAccess(ArrayDomain access, StatementKey statementName)
      : access(access), statementName(statementName) {}

  ArrayDomain access;
  StatementKey statementName;
};

/// Contains all the access from a statement.
class Statement {
 public:
  Statement() {}
  Statement(StatementKey name, std::vector<ArrayAccess> accesses,
            IterDomain iters, ProgDomain prog)
      : statementName(name), accesses_(accesses), iters_(iters), prog_(prog) {}

  StatementKey statementName;
  std::vector<ArrayAccess> accesses_;
  IterDomain iters_;
  ProgDomain prog_;
};

/// The whole program is modeled with Polyhedral.
class PolyhedralModel {
 public:
  PolyhedralModel() {}
  std::vector<Statement> statements_;
};

}  // namespace polly
