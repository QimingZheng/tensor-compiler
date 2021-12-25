#include "ir_check_pass.h"

namespace polly {

bool IRCheckePass::checkInt(IntHandle int_expr) {
  visitInt(int_expr);
  return checkFlag;
}
bool IRCheckePass::checkAdd(AddHandle add) {
  visitAdd(add);
  return checkFlag;
}
bool IRCheckePass::checkMul(MulHandle mul) {
  visitMul(mul);
  return checkFlag;
}
bool IRCheckePass::checkVar(VarHandle var) {
  visitVar(var);
  return checkFlag;
}
bool IRCheckePass::checkAccess(AccessHandle access) {
  visitAccess(access);
  return checkFlag;
}
bool IRCheckePass::checkAssign(AssignmentHandle assign) {
  visitAssign(assign);
  return checkFlag;
}
bool IRCheckePass::checkTensor(TensorHandle tensor) {
  visitTensor(tensor);
  return checkFlag;
}
bool IRCheckePass::checkFor(ForHandle loop) {
  visitFor(loop);
  return checkFlag;
}

void IRCheckAffinePass::visitInt(IntHandle int_expr) {
  // PASS
  isAffine = true;
}
void IRCheckAffinePass::visitAdd(AddHandle add) {
  isAffine = false;
  this->visit(add->lhs);
  if (!isAffine) return;
  isAffine = false;
  this->visit(add->rhs);
  if (!isAffine) return;
  isAffine = true;
}
void IRCheckAffinePass::visitSub(SubHandle sub) {
  isAffine = false;
  this->visit(sub->lhs);
  if (!isAffine) return;
  isAffine = false;
  this->visit(sub->rhs);
  if (!isAffine) return;
  isAffine = true;
}
void IRCheckAffinePass::visitMul(MulHandle mul) {
  isAffine = false;
  this->visit(mul->lhs);
  if (!isAffine) return;
  isAffine = false;
  this->visit(mul->rhs);
  if (!isAffine) return;
  isAffine = true;
}
void IRCheckAffinePass::visitDiv(DivHandle div) {
  isAffine = false;
  this->visit(div->lhs);
  if (!isAffine) return;
  isAffine = false;
  this->visit(div->rhs);
  if (!isAffine) return;
  isAffine = true;
}
void IRCheckAffinePass::visitMod(ModHandle mod) {
  isAffine = false;
  this->visit(mod->lhs);
  if (!isAffine) return;
  isAffine = false;
  this->visit(mod->rhs);
  if (!isAffine) return;
  isAffine = true;
}
void IRCheckAffinePass::visitVar(VarHandle var) {
  // PASS
  isAffine = true;
}
void IRCheckAffinePass::visitAccess(AccessHandle access) {
  containsTensorExpr = false;

  for (int i = 0; i < access->indices.size(); i++) {
    isAffine = false;
    this->visit(access->indices[i]);
    if (!isAffine) return;
    if (containsTensorExpr) {
      isAffine = false;
      return;
    }
  }
  this->visit(access->tensor);
  isAffine = true;
}
void IRCheckAffinePass::visitAssign(AssignmentHandle assign) {
  isAffine = false;
  this->visit(assign->lhs);
  if (!isAffine) return;
  isAffine = false;
  this->visit(assign->rhs);
  if (!isAffine) return;
  isAffine = true;
}
void IRCheckAffinePass::visitTensor(TensorHandle tensor) {
  isAffine = true;
  containsTensorExpr = true;
}
void IRCheckAffinePass::visitFor(ForHandle loop) {
  isAffine = false;
  this->visit(loop->looping_var_);
  if (!isAffine) return;
  for (int i = 0; i < loop->body.size(); i++) {
    isAffine = false;
    this->visit(loop->body[i]);
    if (!isAffine) return;
  }
  isAffine = true;
}
void IRCheckAffinePass::visitConst(ConstHandle con) { isAffine = true; }

void IRConstantBoundaryCheckVisitor::visitInt(IntHandle int_expr) {
  isConstantBoundary = true;
  value = int_expr->value;
}
void IRConstantBoundaryCheckVisitor::visitAdd(AddHandle add) {
  visit(add->lhs);
  if (!isConstantBoundary) return;
  auto lhs = value;
  visit(add->rhs);
  if (!isConstantBoundary) return;
  auto rhs = value;
  value = lhs + rhs;
}
void IRConstantBoundaryCheckVisitor::visitSub(SubHandle sub) {
  visit(sub->lhs);
  if (!isConstantBoundary) return;
  auto lhs = value;
  visit(sub->rhs);
  if (!isConstantBoundary) return;
  auto rhs = value;
  value = lhs - rhs;
}
void IRConstantBoundaryCheckVisitor::visitMul(MulHandle mul) {
  visit(mul->lhs);
  if (!isConstantBoundary) return;
  auto lhs = value;
  visit(mul->rhs);
  if (!isConstantBoundary) return;
  auto rhs = value;
  value = lhs * rhs;
}
void IRConstantBoundaryCheckVisitor::visitDiv(DivHandle div) {
  visit(div->lhs);
  if (!isConstantBoundary) return;
  auto lhs = value;
  visit(div->rhs);
  if (!isConstantBoundary) return;
  auto rhs = value;
  value = lhs / rhs;
}
void IRConstantBoundaryCheckVisitor::visitMod(ModHandle mod) {
  visit(mod->lhs);
  if (!isConstantBoundary) return;
  auto lhs = value;
  visit(mod->rhs);
  if (!isConstantBoundary) return;
  auto rhs = value;
  value = lhs % rhs;
}
void IRConstantBoundaryCheckVisitor::visitVar(VarHandle var) {
  isConstantBoundary = false;
}
void IRConstantBoundaryCheckVisitor::visitAccess(AccessHandle access) {
  isConstantBoundary = false;
}
void IRConstantBoundaryCheckVisitor::visitAssign(AssignmentHandle assign) {
  isConstantBoundary = false;
}
void IRConstantBoundaryCheckVisitor::visitTensor(TensorHandle tensor) {
  isConstantBoundary = false;
}

/// A constant loop boundary must range from 0 to a literal and have an
/// increment equals 1.
void IRConstantBoundaryCheckVisitor::visitFor(ForHandle loop) {
  visit(loop->looping_var_.as<VarNode>()->min);
  if (!isConstantBoundary || value != 0) {
    isConstantBoundary = false;
    return;
  }
  visit(loop->looping_var_.as<VarNode>()->max);
  if (!isConstantBoundary) {
    isConstantBoundary = false;
    return;
  }
  visit(loop->looping_var_.as<VarNode>()->increment);
  if (!isConstantBoundary || value != 1) {
    isConstantBoundary = false;
    return;
  }
  for (int i = 0; i < loop->body.size(); i++) {
    if (loop->body[i].Type() == IRNodeType::FOR) {
      visit(loop->body[i]);
      if (!isConstantBoundary) {
        isConstantBoundary = false;
        return;
      }
    }
  }
  isConstantBoundary = true;
  return;
}
void IRConstantBoundaryCheckVisitor::visitConst(ConstHandle con) {
  isConstantBoundary = false;
}

void IRDivisibleBoundaryCheckVisitor::visitInt(IntHandle int_expr) {
  value = int_expr->value;
}
void IRDivisibleBoundaryCheckVisitor::visitAdd(AddHandle add) {
  visit(add->lhs);
  auto lhs = value;
  visit(add->rhs);
  auto rhs = value;
  value = lhs + rhs;
}
void IRDivisibleBoundaryCheckVisitor::visitSub(SubHandle sub) {
  visit(sub->lhs);
  auto lhs = value;
  visit(sub->rhs);
  auto rhs = value;
  value = lhs - rhs;
}
void IRDivisibleBoundaryCheckVisitor::visitMul(MulHandle mul) {
  visit(mul->lhs);
  auto lhs = value;
  visit(mul->rhs);
  auto rhs = value;
  value = lhs * rhs;
}
void IRDivisibleBoundaryCheckVisitor::visitDiv(DivHandle div) {
  visit(div->lhs);
  auto lhs = value;
  visit(div->rhs);
  auto rhs = value;
  value = lhs / rhs;
}
void IRDivisibleBoundaryCheckVisitor::visitMod(ModHandle mod) {
  visit(mod->lhs);
  auto lhs = value;
  visit(mod->rhs);
  auto rhs = value;
  value = lhs % rhs;
}
void IRDivisibleBoundaryCheckVisitor::visitVar(VarHandle var) {
  /// PASS
  throw std::runtime_error("Should not visit a var node");
}
void IRDivisibleBoundaryCheckVisitor::visitAccess(AccessHandle access) {
  /// PASS
  throw std::runtime_error("Should not visit an access node");
}
void IRDivisibleBoundaryCheckVisitor::visitAssign(AssignmentHandle assign) {
  /// PASS
  throw std::runtime_error("Should not visit an assign node");
}
void IRDivisibleBoundaryCheckVisitor::visitTensor(TensorHandle tensor) {
  /// PASS
  throw std::runtime_error("Should not visit a tensor node");
}
void IRDivisibleBoundaryCheckVisitor::visitFor(ForHandle loop) {
  visit(loop->looping_var_.as<VarNode>()->max);
  if (value % divisor == 0)
    isDivisibleBoundary = true;
  else
    isDivisibleBoundary = false;
}
void IRDivisibleBoundaryCheckVisitor::visitConst(ConstHandle con) {
  /// PASS
  throw std::runtime_error("Should not visit a constant node");
}

}  // namespace polly
