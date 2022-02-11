#include "divisible_boundary_check.h"

namespace polly {

void DivisibleBoundaryCheck::visitInt(IntHandle int_expr) {
  value = int_expr->value;
}

void DivisibleBoundaryCheck::visitFloat(FloatHandle float_expr) {
  throw std::runtime_error("Should not visit a float node");
}
void DivisibleBoundaryCheck::visitAdd(AddHandle add) {
  visit(add->lhs);
  auto lhs = value;
  visit(add->rhs);
  auto rhs = value;
  value = lhs + rhs;
}
void DivisibleBoundaryCheck::visitSub(SubHandle sub) {
  visit(sub->lhs);
  auto lhs = value;
  visit(sub->rhs);
  auto rhs = value;
  value = lhs - rhs;
}
void DivisibleBoundaryCheck::visitMul(MulHandle mul) {
  visit(mul->lhs);
  auto lhs = value;
  visit(mul->rhs);
  auto rhs = value;
  value = lhs * rhs;
}
void DivisibleBoundaryCheck::visitDiv(DivHandle div) {
  visit(div->lhs);
  auto lhs = value;
  visit(div->rhs);
  auto rhs = value;
  value = lhs / rhs;
}
void DivisibleBoundaryCheck::visitMod(ModHandle mod) {
  visit(mod->lhs);
  auto lhs = value;
  visit(mod->rhs);
  auto rhs = value;
  value = lhs % rhs;
}
void DivisibleBoundaryCheck::visitVar(VarHandle var) {
  /// PASS
  throw std::runtime_error("Should not visit a var node");
}
void DivisibleBoundaryCheck::visitAccess(AccessHandle access) {
  /// PASS
  throw std::runtime_error("Should not visit an access node");
}
void DivisibleBoundaryCheck::visitAssign(AssignmentHandle assign) {
  /// PASS
  throw std::runtime_error("Should not visit an assign node");
}
void DivisibleBoundaryCheck::visitTensor(TensorHandle tensor) {
  /// PASS
  throw std::runtime_error("Should not visit a tensor node");
}
void DivisibleBoundaryCheck::visitVal(ValHandle val) {
  /// PASS
  throw std::runtime_error("Should not visit an val node");
}
void DivisibleBoundaryCheck::visitFor(ForHandle loop) {
  visit(loop->looping_var_.as<VarNode>()->max);
  if (value % divisor == 0)
    isDivisibleBoundary = true;
  else
    isDivisibleBoundary = false;
}
void DivisibleBoundaryCheck::visitConst(ConstHandle con) {
  /// PASS
  throw std::runtime_error("Should not visit a constant node");
}
void DivisibleBoundaryCheck::visitPrint(PrintHandle print) {
  /// PASS
  throw std::runtime_error("Should not visit a print node");
}

void DivisibleBoundaryCheck::visitFunc(FuncHandle func) {
  /// PASS
  throw std::runtime_error("Should not visit a func node");
}

void DivisibleBoundaryCheck::visitMin(MinHandle min) {
  visit(min->lhs);
  auto lhs = value;
  visit(min->rhs);
  auto rhs = value;
  value = std::min(lhs, rhs);
}

void DivisibleBoundaryCheck::visitMax(MaxHandle max) {
  visit(max->lhs);
  auto lhs = value;
  visit(max->rhs);
  auto rhs = value;
  value = std::max(lhs, rhs);
}

}  // namespace polly