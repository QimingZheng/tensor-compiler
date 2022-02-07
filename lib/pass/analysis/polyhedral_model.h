/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:29:44
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-28 16:29:55
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

/*!
 * \brief QuasiAffineExpr describes a linear combination of variables. It
 * represents formulas in the form of: (\sum_{i = 1}^{n} c_i x_i + c_0) / d,
 * where c_i is constant integer coefficients, x_i is integer variable.
 */
class QuasiAffineExpr {
 public:
  QuasiAffineExpr() { clear(); }
  QuasiAffineExpr(std::map<VarKey, int> coeffs, int constant, int divisor)
      : coeffs(coeffs), constant(constant), divisor(divisor) {}

  QuasiAffineExpr(const QuasiAffineExpr& other)
      : coeffs(other.coeffs),
        constant(other.constant),
        divisor(other.divisor) {}

  QuasiAffineExpr& operator=(const QuasiAffineExpr& other) {
    coeffs = other.coeffs;
    constant = other.constant;
    divisor = other.divisor;
  }

  void clear() {
    coeffs.clear();
    constant = 0;
    divisor = 1;
  }

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

/*!
 * \brief Iteration describes one for loop through the boundary of the
 * corresponding iteration variable.
 */
class Iteration {
 public:
  Iteration() {}
  // Iteration(VarKey iter_name, QuasiAffineExpr lower, QuasiAffineExpr upper)
  //     : iterName_(iter_name), lowerBound_(lower), upperBound_(upper) {}
  Iteration(VarKey iter_name, std::vector<QuasiAffineExpr> lower,
            std::vector<QuasiAffineExpr> upper)
      : iterName_(iter_name), lowerBounds_(lower), upperBounds_(upper) {}

  VarKey iterName_;
  // TODO: make the lowerBounds and upperBounds accept multiple
  // affine-expressions.
  std::vector<QuasiAffineExpr> lowerBounds_;
  std::vector<QuasiAffineExpr> upperBounds_;

  // QuasiAffineExpr lowerBound_;
  // QuasiAffineExpr upperBound_;
  // increment is always +1.

  std::string DbgMsg() {
    std::string ret = "";
    for (auto expr : lowerBounds_) {
      ret += expr.DbgMsg() + " <= " + iterName_ + "\n";
    }
    for (auto expr : upperBounds_) {
      ret += iterName_ + " < " + expr.DbgMsg() + "\n";
    }
    return ret;
  }
};

/*!
 * \brief The iteration domain describe a polyhedral constructed by several for
 * loop iteration varaibles. Usually used for describing the space of a
 * statement.
 */
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

/*!
 * \brief ReorderedBounds encapsulates the helper functions to obtain the
 * boundary after the reordering transformation.
 */
class ReorderedBounds {
 public:
  ReorderedBounds() {}

  void GetReorderedBound(std::vector<IRHandle> loop_vars,
                         std::vector<Iteration> extractedIters, int ith,
                         int jth);

 private:
  IRHandle IslConstraintToBound(solver::constraint c, std::string loop_name);
  IRHandle QuasiAffineExprToIR(QuasiAffineExpr expr);

  std::vector<IRHandle> loop_vars;
  std::vector<Iteration> extractedIters;
  int ith;
  int jth;
};

class EmptyBounds {
 public:
  static bool IsEmptyPolyhedral(std::vector<IRHandle> loop_vars,
                                std::vector<Iteration> extractedIters);
};

/*!
 * \brief ProgDomain describe the context of a statement inside a program.
 */
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

/*!
 * \brief ArrayDomain describe one array access from one statement, including
 * the accessed array name, read/write type, affine-map to the array indices.
 */
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

/*!
 * \brief  An array access instance is an array access from a certain statement.
 */
class ArrayAccess {
 public:
  ArrayAccess(ArrayDomain access, StatementKey statementName)
      : access(access), statementName(statementName) {}

  ArrayDomain access;
  StatementKey statementName;
  std::string DbgMsg() { return access.DbgMsg(); }
};

/*!
 * \brief A Statement describeds all the array access from of statement.
 */
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

/*!
 * \brief The whole program is modeled as a Polyhedral Model by expressing every
 * statetment in the program with affine expresions.
 */
class PolyhedralModel {
 public:
  PolyhedralModel() {}
  PolyhedralModel operator+(PolyhedralModel model) {
    PolyhedralModel ret;
    ret.statements_ = statements_;
    ret.statements_.insert(ret.statements_.begin(), model.statements_.begin(),
                           model.statements_.end());
    return ret;
  }
  std::vector<Statement> statements_;
};

}  // namespace polly
