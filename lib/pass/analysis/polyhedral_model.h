/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:29:44
 * @Last Modified by:   Qiming Zheng
 * @Last Modified time: 2022-01-18 20:29:44
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_module.h"
#include "solver/solver.h"

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

  std::string DbgMsg() {
    std::string ret = "";
    for (auto it : coeffs) {
      ret += std::to_string(it.second) + "*" + it.first + " + ";
    }
    ret += std::to_string(constant);
    if (divisor != 1) {
      ret = "(" + ret + ") / " + std::to_string(divisor);
    }
    return ret;
  }
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

  std::string DbgMsg() {
    return lowerBound_.DbgMsg() + " <= " + iterName_ + " < " +
           upperBound_.DbgMsg();
  }
};

class IterDomain {
 public:
  IterDomain() {}
  IterDomain(std::vector<Iteration> iterations) : iterations_(iterations) {}
  std::vector<std::string> GerIters() {
    std::vector<std::string> ret;
    for (int i = 0; i < iterations_.size(); i++) {
      ret.push_back(iterations_[i].iterName_);
    }
    return ret;
  }
  std::vector<Iteration> iterations_;

  std::string DbgMsg() {
    std::string ret = "";
    for (int i = 0; i < iterations_.size(); i++) {
      ret += iterations_[i].DbgMsg() + "\n";
    }
    return ret;
  }
};

class ReorderedBounds {
 public:
  ReorderedBounds() {}

  void GetReorderedBound(std::vector<IRHandle> loop_vars,
                         std::vector<Iteration> extractedIters, int ith,
                         int jth);

 private:
  IRHandle IslConstraintToIR(solver::constraint c);
  IRHandle QuasiAffineExprToIR(QuasiAffineExpr expr);

  std::vector<IRHandle> loop_vars;
  std::vector<Iteration> extractedIters;
  int ith;
  int jth;
};

class ProgDomain {
 public:
  ProgDomain() {}
  ProgDomain(std::vector<int> program_context)
      : progContext_(program_context) {}

  std::vector<int> progContext_;
  std::string DbgMsg() {
    std::string ret = "";
    for (int i = 0; i < progContext_.size(); i++) {
      ret += std::to_string(progContext_[i]) + ", ";
    }
    ret = ret.substr(0, ret.length() - 2);
    return ret;
  }
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
  std::string DbgMsg() {
    std::string ret = arrayName_;
    for (int i = 0; i < indices_.size(); i++) {
      ret += "[" + indices_[i].DbgMsg() + "]";
    }
    return ret;
  }
};

/// An array access instance is an array access from a certain statement.
class ArrayAccess {
 public:
  ArrayAccess(ArrayDomain access, StatementKey statementName)
      : access(access), statementName(statementName) {}

  ArrayDomain access;
  StatementKey statementName;
  std::string DbgMsg() { return access.DbgMsg(); }
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

  std::string DbgMsg() {
    std::string ret = statementName + ":\n";
    ret += prog_.DbgMsg() + "\n";
    ret += iters_.DbgMsg() + "\n";
    for (int i = 0; i < accesses_.size(); i++) {
      ret += accesses_[i].DbgMsg() + "\n";
    }
    return ret;
  }
};

/// The whole program is modeled with Polyhedral.
class PolyhedralModel {
 public:
  PolyhedralModel() {}
  std::vector<Statement> statements_;
};

}  // namespace polly
