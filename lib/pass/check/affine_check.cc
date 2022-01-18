#include "affine_check.h"

namespace polly {

void AffineCheck::visitInt(IntHandle int_expr) {
  // PASS
  isAffine = true;
}
void AffineCheck::visitAdd(AddHandle add) {
  isAffine = false;
  this->visit(add->lhs);
  if (!isAffine) return;
  isAffine = false;
  this->visit(add->rhs);
  if (!isAffine) return;
  isAffine = true;
}
void AffineCheck::visitSub(SubHandle sub) {
  isAffine = false;
  this->visit(sub->lhs);
  if (!isAffine) return;
  isAffine = false;
  this->visit(sub->rhs);
  if (!isAffine) return;
  isAffine = true;
}
void AffineCheck::visitMul(MulHandle mul) {
  isAffine = false;
  this->visit(mul->lhs);
  if (!isAffine) return;
  isAffine = false;
  this->visit(mul->rhs);
  if (!isAffine) return;
  isAffine = true;
}
void AffineCheck::visitDiv(DivHandle div) {
  isAffine = false;
  this->visit(div->lhs);
  if (!isAffine) return;
  isAffine = false;
  this->visit(div->rhs);
  if (!isAffine) return;
  isAffine = true;
}
void AffineCheck::visitMod(ModHandle mod) {
  isAffine = false;
  this->visit(mod->lhs);
  if (!isAffine) return;
  isAffine = false;
  this->visit(mod->rhs);
  if (!isAffine) return;
  isAffine = true;
}
void AffineCheck::visitVar(VarHandle var) {
  // PASS
  isAffine = true;
}
void AffineCheck::visitAccess(AccessHandle access) {
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
void AffineCheck::visitAssign(AssignmentHandle assign) {
  isAffine = false;
  this->visit(assign->lhs);
  if (!isAffine) return;
  isAffine = false;
  this->visit(assign->rhs);
  if (!isAffine) return;
  isAffine = true;
}
void AffineCheck::visitTensor(TensorHandle tensor) {
  isAffine = true;
  containsTensorExpr = true;
}
void AffineCheck::visitFor(ForHandle loop) {
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

void AffineCheck::visitConst(ConstHandle con) { isAffine = true; }

void AffineCheck::visitPrint(PrintHandle print) {
  // TODO: check the content being printed maybe.
  isAffine = true;
}

void AffineCheck::visitFunc(FuncHandle func) {
  for (int i = 0; i < func->body.size(); i++) {
    func->body[i].accept(this);
    if (!isAffine) return;
  }
  isAffine = true;
}
}