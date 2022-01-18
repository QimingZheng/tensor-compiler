/*
 * Created Date: 17th January 2022
 * Author: Qiming Zheng
 * Copyright (c) 2022 Qiming Zheng
 */

#include "polyhedral_extraction.h"

namespace polly {

void PolyhedralExtraction::visitInt(IntHandle int_expr) {
  expr.clear();
  expr.constant = int_expr->value;
}

void PolyhedralExtraction::visitAdd(AddHandle add) {
  expr.clear();
  add->lhs.accept(this);
  auto lhs = expr;
  add->rhs.accept(this);
  auto rhs = expr;
  expr.clear();
  for (auto &it : lhs.coeffs) {
    it.second *= rhs.divisor;
  }
  for (auto &it : rhs.coeffs) {
    it.second *= lhs.divisor;
  }
  for (auto it : lhs.coeffs) {
    if (rhs.coeffs.find(it.first) != rhs.coeffs.end()) {
      lhs.coeffs[it.first] += rhs.coeffs[it.first];
    }
  }
  for (auto it : rhs.coeffs) {
    if (lhs.coeffs.find(it.first) == lhs.coeffs.end()) {
      lhs.coeffs[it.first] = it.second;
    }
  }
  expr = lhs;
  expr.divisor = lhs.divisor * rhs.divisor;
  expr.constant = lhs.constant * rhs.divisor + rhs.constant * lhs.divisor;
}

void PolyhedralExtraction::visitSub(SubHandle sub) {
  expr.clear();
  sub->lhs.accept(this);
  auto lhs = expr;
  sub->rhs.accept(this);
  auto rhs = expr;
  expr.clear();
  for (auto &it : lhs.coeffs) {
    it.second *= rhs.divisor;
  }
  for (auto &it : rhs.coeffs) {
    it.second *= lhs.divisor;
  }
  for (auto it : lhs.coeffs) {
    if (rhs.coeffs.find(it.first) != rhs.coeffs.end()) {
      lhs.coeffs[it.first] -= rhs.coeffs[it.first];
    }
  }
  for (auto it : rhs.coeffs) {
    if (lhs.coeffs.find(it.first) == lhs.coeffs.end()) {
      lhs.coeffs[it.first] = -it.second;
    }
  }
  expr = lhs;
  expr.divisor = lhs.divisor * rhs.divisor;
  expr.constant = lhs.constant * rhs.divisor - rhs.constant * lhs.divisor;
}

void PolyhedralExtraction::visitMul(MulHandle mul) {
  expr.clear();
  mul->lhs.accept(this);
  auto lhs = expr;
  mul->rhs.accept(this);
  auto rhs = expr;
  expr.clear();
  if (lhs.coeffs.size() == 0) {
    rhs.divisor *= lhs.divisor;
    for (auto &it : rhs.coeffs) {
      it.second *= lhs.constant;
    }
    rhs.constant *= lhs.constant;
    expr = rhs;
  } else {
    lhs.divisor *= rhs.divisor;
    for (auto &it : lhs.coeffs) {
      it.second *= rhs.constant;
    }
    lhs.constant *= rhs.constant;
    expr = lhs;
  }
}

void PolyhedralExtraction::visitDiv(DivHandle div) {
  /// Pass
  expr.clear();
  div->lhs.accept(this);
  auto lhs = expr;
  div->rhs.accept(this);
  auto rhs = expr;
  expr.clear();
  lhs.divisor *= rhs.constant;
  for (auto &it : lhs.coeffs) {
    it.second *= rhs.divisor;
  }
  lhs.constant *= rhs.divisor;
  expr = lhs;
}

void PolyhedralExtraction::visitMod(ModHandle mod) {
  /// Pass
  throw std::runtime_error(
      "Mod operation is not supported by the isl solver currently");
}

void PolyhedralExtraction::visitVar(VarHandle var) {
  expr.clear();
  expr.coeffs[var->id] = 1;
}

void PolyhedralExtraction::visitAccess(AccessHandle access) {
  // a placeholder
  std::vector<QuasiAffineExpr> placeholder;
  for (int i = 0; i < access->indices.size(); i++) {
    expr.clear();
    access->indices[i].accept(this);
    placeholder.push_back(expr);
  }
  affineAccesses.push_back({access->tensor.as<TensorNode>()->id, placeholder});
}

void PolyhedralExtraction::visitAssign(AssignmentHandle assign) {
  StatementKey statementName = assign->id;
  std::vector<ArrayAccess> accesses;

  affineAccesses.clear();
  assign->lhs.accept(this);
  for (int i = 0; i < affineAccesses.size(); i++) {
    accesses.push_back(ArrayAccess(
        ArrayDomain(ArrayDomain::AccessType::WRITE, affineAccesses[i].first,
                    affineAccesses[i].second),
        statementName));
  }
  affineAccesses.clear();
  assign->rhs.accept(this);
  for (int i = 0; i < affineAccesses.size(); i++) {
    accesses.push_back(ArrayAccess(
        ArrayDomain(ArrayDomain::AccessType::READ, affineAccesses[i].first,
                    affineAccesses[i].second),
        statementName));
  }

  model.statements_.push_back(Statement(
      statementName, accesses, IterDomain(loops), ProgDomain(progContext)));
}

void PolyhedralExtraction::visitTensor(TensorHandle tensor) {
  /// Pass
}

void PolyhedralExtraction::visitFor(ForHandle loop) {
  expr.clear();
  loop->looping_var_.as<VarNode>()->min.accept(this);
  auto min_ = expr;
  expr.clear();
  loop->looping_var_.as<VarNode>()->max.accept(this);
  auto max_ = expr;

  loops.push_back(Iteration(loop->looping_var_.as<VarNode>()->id, min_, max_));
  for (int i = 0; i < loop->body.size(); i++) {
    progContext.push_back(i);
    loop->body[i].accept(this);
    progContext.pop_back();
  }
  loops.pop_back();
}

void PolyhedralExtraction::visitConst(ConstHandle con) {
  /// Pass
}

void PolyhedralExtraction::visitPrint(PrintHandle print) {
  /// Pass
  StatementKey statementName = print->id;

  std::vector<ArrayAccess> accesses;

  affineAccesses.clear();
  print->print.accept(this);
  for (int i = 0; i < affineAccesses.size(); i++) {
    accesses.push_back(ArrayAccess(
        ArrayDomain(ArrayDomain::AccessType::READ, affineAccesses[i].first,
                    affineAccesses[i].second),
        statementName));
  }

  model.statements_.push_back(Statement(
      statementName, accesses, IterDomain(loops), ProgDomain(progContext)));
}

void PolyhedralExtraction::visitFunc(FuncHandle func) {
  progContext.clear();
  for (int i = 0; i < func->body.size(); i++) {
    progContext.push_back(i);
    func->body[i].accept(this);
    progContext.pop_back();
  }
}

}  // namespace polly