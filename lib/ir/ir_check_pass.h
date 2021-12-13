#pragma once

#include "ir.h"
#include "ir_visitor.h"

namespace polly {

class IRCheckePass : public IRVisitor {
 public:
  bool checkFlag = false;
  bool checkInt(IntNode *int_expr) {
    visitInt(int_expr);
    return checkFlag;
  }
  bool checkAdd(AddNode *add) {
    visitAdd(add);
    return checkFlag;
  }
  bool checkMul(MulNode *mul) {
    visitMul(mul);
    return checkFlag;
  }
  bool checkVar(VarNode *var) {
    visitVar(var);
    return checkFlag;
  }
  bool checkAccess(AccessNode *access) {
    visitAccess(access);
    return checkFlag;
  }
  bool checkAssign(AssignmentNode *assign) {
    visitAssign(assign);
    return checkFlag;
  }
  bool checkTensor(TensorNode *tensor) {
    visitTensor(tensor);
    return checkFlag;
  }
  bool checkFor(ForNode *loop) {
    visitFor(loop);
    return checkFlag;
  }
};

class IRCheckAffinePass : public IRCheckePass {
 public:
  bool &isAffine;
  bool containsTensorExpr;
  IRCheckAffinePass() : isAffine(checkFlag), containsTensorExpr(false) {}

  void visitInt(IntNode *int_expr) override {
    // PASS
    isAffine = true;
  }
  void visitAdd(AddNode *add) override {
    isAffine = false;
    this->visit(add->lhs);
    if (!isAffine) return;
    isAffine = false;
    this->visit(add->rhs);
    if (!isAffine) return;
    isAffine = true;
  }
  void visitMul(MulNode *mul) override {
    isAffine = false;
    this->visit(mul->lhs);
    if (!isAffine) return;
    isAffine = false;
    this->visit(mul->rhs);
    if (!isAffine) return;
    isAffine = true;
  }
  void visitVar(VarNode *var) override {
    // PASS
    isAffine = true;
  }
  void visitAccess(AccessNode *access) override {
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
  void visitAssign(AssignmentNode *assign) override {
    isAffine = false;
    this->visit(assign->lhs);
    if (!isAffine) return;
    isAffine = false;
    this->visit(assign->rhs);
    if (!isAffine) return;
    isAffine = true;
  }
  void visitTensor(TensorNode *tensor) override {
    isAffine = true;
    containsTensorExpr = true;
  }
  void visitFor(ForNode *loop) override {
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
};

}  // namespace polly
