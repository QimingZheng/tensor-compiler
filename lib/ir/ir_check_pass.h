#pragma once

#include "ir.h"
#include "ir_visitor.h"

namespace polly {

class IRCheckePass : public IRVisitor {
 public:
  bool checkFlag = false;
  bool checkInt(IntHandle int_expr) {
    visitInt(int_expr);
    return checkFlag;
  }
  bool checkAdd(AddHandle add) {
    visitAdd(add);
    return checkFlag;
  }
  bool checkMul(MulHandle mul) {
    visitMul(mul);
    return checkFlag;
  }
  bool checkVar(VarHandle var) {
    visitVar(var);
    return checkFlag;
  }
  bool checkAccess(AccessHandle access) {
    visitAccess(access);
    return checkFlag;
  }
  bool checkAssign(AssignmentHandle assign) {
    visitAssign(assign);
    return checkFlag;
  }
  bool checkTensor(TensorHandle tensor) {
    visitTensor(tensor);
    return checkFlag;
  }
  bool checkFor(ForHandle loop) {
    visitFor(loop);
    return checkFlag;
  }
};

class IRCheckAffinePass : public IRCheckePass {
 public:
  bool &isAffine;
  bool containsTensorExpr;
  IRCheckAffinePass() : isAffine(checkFlag), containsTensorExpr(false) {}

  void visitInt(IntHandle int_expr) override {
    // PASS
    isAffine = true;
  }
  void visitAdd(AddHandle add) override {
    isAffine = false;
    this->visit(add->lhs);
    if (!isAffine) return;
    isAffine = false;
    this->visit(add->rhs);
    if (!isAffine) return;
    isAffine = true;
  }
  void visitSub(SubHandle sub) override {
    isAffine = false;
    this->visit(sub->lhs);
    if (!isAffine) return;
    isAffine = false;
    this->visit(sub->rhs);
    if (!isAffine) return;
    isAffine = true;
  }
  void visitMul(MulHandle mul) override {
    isAffine = false;
    this->visit(mul->lhs);
    if (!isAffine) return;
    isAffine = false;
    this->visit(mul->rhs);
    if (!isAffine) return;
    isAffine = true;
  }
  void visitDiv(DivHandle div) override {
    isAffine = false;
    this->visit(div->lhs);
    if (!isAffine) return;
    isAffine = false;
    this->visit(div->rhs);
    if (!isAffine) return;
    isAffine = true;
  }
  void visitMod(ModHandle mod) override {
    isAffine = false;
    this->visit(mod->lhs);
    if (!isAffine) return;
    isAffine = false;
    this->visit(mod->rhs);
    if (!isAffine) return;
    isAffine = true;
  }
  void visitVar(VarHandle var) override {
    // PASS
    isAffine = true;
  }
  void visitAccess(AccessHandle access) override {
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
  void visitAssign(AssignmentHandle assign) override {
    isAffine = false;
    this->visit(assign->lhs);
    if (!isAffine) return;
    isAffine = false;
    this->visit(assign->rhs);
    if (!isAffine) return;
    isAffine = true;
  }
  void visitTensor(TensorHandle tensor) override {
    isAffine = true;
    containsTensorExpr = true;
  }
  void visitFor(ForHandle loop) override {
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
  void visitConst(ConstHandle con) override { isAffine = true; }
};

}  // namespace polly
