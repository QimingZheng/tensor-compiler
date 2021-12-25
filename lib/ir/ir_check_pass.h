#pragma once

#include "ir.h"
#include "ir_visitor.h"

namespace polly {

class IRCheckePass : public IRVisitor {
 public:
  bool checkFlag = false;

  virtual bool checkInt(IntHandle int_expr);
  virtual bool checkAdd(AddHandle add);
  virtual bool checkMul(MulHandle mul);
  virtual bool checkVar(VarHandle var);
  virtual bool checkAccess(AccessHandle access);
  virtual bool checkAssign(AssignmentHandle assign);
  virtual bool checkTensor(TensorHandle tensor);
  virtual bool checkFor(ForHandle loop);
};

class IRCheckAffinePass : public IRCheckePass {
 public:
  bool &isAffine;
  bool containsTensorExpr;
  IRCheckAffinePass() : isAffine(checkFlag), containsTensorExpr(false) {}

  void visitInt(IntHandle int_expr) override;
  void visitAdd(AddHandle add) override;
  void visitSub(SubHandle sub) override;
  void visitMul(MulHandle mul) override;
  void visitDiv(DivHandle div) override;
  void visitMod(ModHandle mod) override;
  void visitVar(VarHandle var) override;
  void visitAccess(AccessHandle access) override;
  void visitAssign(AssignmentHandle assign) override;
  void visitTensor(TensorHandle tensor) override;
  void visitFor(ForHandle loop) override;
  void visitConst(ConstHandle con) override;
};

class IRConstantBoundaryCheckVisitor : public IRCheckePass {
 public:
  bool &isConstantBoundary;
  int value = -1;
  IRConstantBoundaryCheckVisitor() : isConstantBoundary(checkFlag) {}

  void visitInt(IntHandle int_expr) override;
  void visitAdd(AddHandle add) override;
  void visitSub(SubHandle sub) override;
  void visitMul(MulHandle mul) override;
  void visitDiv(DivHandle div) override;
  void visitMod(ModHandle mod) override;
  void visitVar(VarHandle var) override;
  void visitAccess(AccessHandle access) override;
  void visitAssign(AssignmentHandle assign) override;
  void visitTensor(TensorHandle tensor) override;
  void visitFor(ForHandle loop) override;
  void visitConst(ConstHandle con) override;
};

/// IRDivisibleBoundaryCheckVisitor checks whether the boundary of a loop can be
/// divided by a certain divisor. Please make sure that the boundary of it is
/// checked by the IRConstantBoundaryCheckVisitor checker first.
class IRDivisibleBoundaryCheckVisitor : public IRCheckePass {
 public:
  bool &isDivisibleBoundary;
  int divisor;
  int value = -1;
  IRDivisibleBoundaryCheckVisitor(int divisor)
      : divisor(divisor), isDivisibleBoundary(checkFlag) {}

  void visitInt(IntHandle int_expr) override;
  void visitAdd(AddHandle add) override;
  void visitSub(SubHandle sub) override;
  void visitMul(MulHandle mul) override;
  void visitDiv(DivHandle div) override;
  void visitMod(ModHandle mod) override;
  void visitVar(VarHandle var) override;
  void visitAccess(AccessHandle access) override;
  void visitAssign(AssignmentHandle assign) override;
  void visitTensor(TensorHandle tensor) override;
  void visitFor(ForHandle loop) override;
  void visitConst(ConstHandle con) override;
};

}  // namespace polly
