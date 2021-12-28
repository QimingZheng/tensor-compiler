#pragma once

#include "common.h"
#include "check_pass.h"

namespace polly {

class ConstantBoundaryCheck : public CheckPass, public IRVisitor {
 public:
  bool isConstantBoundary;
  int value = -1;
  ConstantBoundaryCheck(IRHandle program) : program_(program) {}

  bool Check() override {
    this->visit(program_);
    return isConstantBoundary;
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

 private:
  IRHandle program_;
};

}  // namespace polly