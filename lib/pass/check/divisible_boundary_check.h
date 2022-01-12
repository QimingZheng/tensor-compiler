#pragma once

#include "common.h"
#include "check_pass.h"

namespace polly {

/// DivisibleBoundaryCheck checks whether the boundary of a loop can be
/// divided by a certain divisor. Please make sure that the boundary of it is
/// checked by the IRConstantBoundaryCheckVisitor checker first.
class DivisibleBoundaryCheck : public CheckPass, public IRVisitor {
 public:
  bool isDivisibleBoundary;
  int divisor;
  int value = -1;
  DivisibleBoundaryCheck(IRHandle loop, int divisor)
      : loop_(loop), divisor(divisor) {}

  bool Check() override {
    this->visit(loop_);
    return isDivisibleBoundary;
  }

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
  void visitPrint(PrintHandle print) override;
  void visitFunc(FuncHandle func) override;

 private:
  IRHandle loop_;
};
}  // namespace polly