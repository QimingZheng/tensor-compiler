#include "constant_boundary_check.h"

namespace polly {

void ConstantBoundaryCheck::visitInt(IntHandle int_expr) {
  isConstantBoundary = true;
  value = int_expr->value;
}
void ConstantBoundaryCheck::visitFloat(FloatHandle float_expr) {
  isConstantBoundary = false;
}
void ConstantBoundaryCheck::visitAdd(AddHandle add) {
  visit(add->lhs);
  if (!isConstantBoundary) return;
  auto lhs = value;
  visit(add->rhs);
  if (!isConstantBoundary) return;
  auto rhs = value;
  value = lhs + rhs;
}
void ConstantBoundaryCheck::visitSub(SubHandle sub) {
  visit(sub->lhs);
  if (!isConstantBoundary) return;
  auto lhs = value;
  visit(sub->rhs);
  if (!isConstantBoundary) return;
  auto rhs = value;
  value = lhs - rhs;
}
void ConstantBoundaryCheck::visitMul(MulHandle mul) {
  visit(mul->lhs);
  if (!isConstantBoundary) return;
  auto lhs = value;
  visit(mul->rhs);
  if (!isConstantBoundary) return;
  auto rhs = value;
  value = lhs * rhs;
}
void ConstantBoundaryCheck::visitDiv(DivHandle div) {
  visit(div->lhs);
  if (!isConstantBoundary) return;
  auto lhs = value;
  visit(div->rhs);
  if (!isConstantBoundary) return;
  auto rhs = value;
  value = lhs / rhs;
}
void ConstantBoundaryCheck::visitMod(ModHandle mod) {
  visit(mod->lhs);
  if (!isConstantBoundary) return;
  auto lhs = value;
  visit(mod->rhs);
  if (!isConstantBoundary) return;
  auto rhs = value;
  value = lhs % rhs;
}
void ConstantBoundaryCheck::visitVar(VarHandle var) {
  isConstantBoundary = false;
}
void ConstantBoundaryCheck::visitAccess(AccessHandle access) {
  isConstantBoundary = false;
}
void ConstantBoundaryCheck::visitAssign(AssignmentHandle assign) {
  isConstantBoundary = false;
}
void ConstantBoundaryCheck::visitTensor(TensorHandle tensor) {
  isConstantBoundary = false;
}

void ConstantBoundaryCheck::visitVal(ValHandle val) {
  isConstantBoundary = false;
}

/// A constant loop boundary must range from 0 to a literal and have an
/// increment equals 1.
void ConstantBoundaryCheck::visitFor(ForHandle loop) {
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
void ConstantBoundaryCheck::visitConst(ConstHandle con) {
  isConstantBoundary = false;
}

void ConstantBoundaryCheck::visitPrint(PrintHandle print) {
  // TODO: check whether this is true.
  isConstantBoundary = true;
}

void ConstantBoundaryCheck::visitFunc(FuncHandle func) {
  for (int i = 0; i < func->body.size(); i++) {
    func->body[i].accept(this);
    if (!isConstantBoundary) {
      return;
    }
  }
  isConstantBoundary = true;
  return;
}

void ConstantBoundaryCheck::visitMin(MinHandle min) {
  visit(min->lhs);
  if (!isConstantBoundary) return;
  auto lhs = value;
  visit(min->rhs);
  if (!isConstantBoundary) return;
  auto rhs = value;
  value = std::min(lhs, rhs);
}

void ConstantBoundaryCheck::visitMax(MaxHandle max) {
  visit(max->lhs);
  if (!isConstantBoundary) return;
  auto lhs = value;
  visit(max->rhs);
  if (!isConstantBoundary) return;
  auto rhs = value;
  value = std::max(lhs, rhs);
}

}  // namespace polly
