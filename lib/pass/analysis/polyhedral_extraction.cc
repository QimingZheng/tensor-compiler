/*
 * Created Date: 17th January 2022
 * Author: Qiming Zheng
 * Copyright (c) 2022 Qiming Zheng
 */

#include "polyhedral_extraction.h"

namespace polly {

void PolyhedralExtraction::visitInt(IntHandle int_expr) {
  workspace.clear();
  workspace.expr.constant = int_expr->value;
}

void PolyhedralExtraction::visitFloat(FloatHandle float_expr) {
  workspace.clear();
  // throw std::runtime_error(
  //     "Shall not visit a float node when extract polyhedral model");
}

void PolyhedralExtraction::visitAdd(AddHandle add) {
  workspace.clear();
  add->lhs.accept(this);
  auto lhs = workspace.expr;
  add->rhs.accept(this);
  auto rhs = workspace.expr;
  workspace.clear();
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
  workspace.expr = lhs;
  workspace.expr.divisor = lhs.divisor * rhs.divisor;
  workspace.expr.constant =
      lhs.constant * rhs.divisor + rhs.constant * lhs.divisor;
}

void PolyhedralExtraction::visitSub(SubHandle sub) {
  workspace.clear();
  sub->lhs.accept(this);
  auto lhs = workspace.expr;
  sub->rhs.accept(this);
  auto rhs = workspace.expr;
  workspace.clear();
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
  workspace.expr = lhs;
  workspace.expr.divisor = lhs.divisor * rhs.divisor;
  workspace.expr.constant =
      lhs.constant * rhs.divisor - rhs.constant * lhs.divisor;
}

void PolyhedralExtraction::visitMul(MulHandle mul) {
  workspace.clear();
  mul->lhs.accept(this);
  auto lhs = workspace.expr;
  mul->rhs.accept(this);
  auto rhs = workspace.expr;
  workspace.clear();
  if (lhs.coeffs.size() == 0) {
    rhs.divisor *= lhs.divisor;
    for (auto &it : rhs.coeffs) {
      it.second *= lhs.constant;
    }
    rhs.constant *= lhs.constant;
    workspace.expr = rhs;
  } else {
    lhs.divisor *= rhs.divisor;
    for (auto &it : lhs.coeffs) {
      it.second *= rhs.constant;
    }
    lhs.constant *= rhs.constant;
    workspace.expr = lhs;
  }
}

void PolyhedralExtraction::visitDiv(DivHandle div) {
  /// Pass
  workspace.clear();
  div->lhs.accept(this);
  auto lhs = workspace.expr;
  div->rhs.accept(this);
  auto rhs = workspace.expr;
  workspace.clear();
  lhs.divisor *= rhs.constant;
  for (auto &it : lhs.coeffs) {
    it.second *= rhs.divisor;
  }
  lhs.constant *= rhs.divisor;
  workspace.expr = lhs;
}

void PolyhedralExtraction::visitMod(ModHandle mod) {
  /// Pass
  throw std::runtime_error(
      "Mod operation is not supported by the isl solver currently");
}

void PolyhedralExtraction::visitVar(VarHandle var) {
  workspace.clear();
  workspace.expr.coeffs[var->id] = 1;
}

void PolyhedralExtraction::visitAccess(AccessHandle access) {
  // a placeholder
  std::vector<QuasiAffineExpr> placeholder;
  for (int i = 0; i < access->indices.size(); i++) {
    workspace.clear();
    access->indices[i].accept(this);
    placeholder.push_back(workspace.expr);
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

void PolyhedralExtraction::visitVal(ValHandle val) {
  std::vector<QuasiAffineExpr> placeholder;
  for (int i = 0; i < val->enclosing_looping_vars_.size(); i++) {
    workspace.clear();
    val->enclosing_looping_vars_[i].accept(this);
    placeholder.push_back(workspace.expr);
  }
  affineAccesses.push_back({val->id, placeholder});
}

void PolyhedralExtraction::visitDecl(DeclHandle decl) {
  /// Pass
}

void PolyhedralExtraction::visitFor(ForHandle loop) {
  workspace.clear();
  loop->looping_var_.as<VarNode>()->min.accept(this);
  auto min_ws = workspace;
  auto mins_ = workspace.max_exprs.size() > 0
                   ? workspace.max_exprs
                   : std::vector<QuasiAffineExpr>{workspace.expr};
  workspace.clear();
  loop->looping_var_.as<VarNode>()->max.accept(this);
  auto max_ws = workspace;
  auto maxs_ = workspace.min_exprs.size() > 0
                   ? workspace.min_exprs
                   : std::vector<QuasiAffineExpr>{workspace.expr};

  for (auto &max_ : maxs_) {
    // max_ is a non-inclusive bound
    // max_.constant -= 1;
    max_.constant -= 1 * max_.divisor;
  }

  loops.push_back(
      Iteration(loop->looping_var_.as<VarNode>()->id, mins_, maxs_));
  for (int i = 0; i < loop->body.size(); i++) {
    progContext.push_back(i);
    loop->body[i].accept(this);
    progContext.pop_back();
  }
  loops.pop_back();
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

void PolyhedralExtraction::visitMin(MinHandle min) {
  workspace.clear();
  min->lhs.accept(this);
  auto lhs_ws = workspace;
  min->rhs.accept(this);
  auto rhs_ws = workspace;
  workspace.clear();

  assert(lhs_ws.max_exprs.size() == 0);
  assert(rhs_ws.max_exprs.size() == 0);
  workspace.min_exprs.insert(workspace.min_exprs.end(),
                             lhs_ws.min_exprs.begin(), lhs_ws.min_exprs.end());
  workspace.min_exprs.insert(workspace.min_exprs.end(),
                             rhs_ws.min_exprs.begin(), rhs_ws.min_exprs.end());
  if (lhs_ws.min_exprs.size() == 0) workspace.min_exprs.push_back(lhs_ws.expr);
  if (rhs_ws.min_exprs.size() == 0) workspace.min_exprs.push_back(rhs_ws.expr);
}

void PolyhedralExtraction::visitMax(MaxHandle max) {
  workspace.clear();
  max->lhs.accept(this);
  auto lhs_ws = workspace;
  max->rhs.accept(this);
  auto rhs_ws = workspace;
  workspace.clear();

  assert(lhs_ws.min_exprs.size() == 0);
  assert(rhs_ws.min_exprs.size() == 0);
  workspace.max_exprs.insert(workspace.max_exprs.end(),
                             lhs_ws.max_exprs.begin(), lhs_ws.max_exprs.end());
  workspace.max_exprs.insert(workspace.max_exprs.end(),
                             rhs_ws.max_exprs.begin(), rhs_ws.max_exprs.end());
  if (lhs_ws.max_exprs.size() == 0) workspace.max_exprs.push_back(lhs_ws.expr);
  if (rhs_ws.max_exprs.size() == 0) workspace.max_exprs.push_back(rhs_ws.expr);
}

}  // namespace polly