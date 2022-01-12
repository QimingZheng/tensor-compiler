#pragma once

#include "common.h"
#include "optimization_pass.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

namespace polly {

class ConstantFoldingPass : public OptimizationPass, public IRVisitor {
 public:
  ConstantFoldingPass(IRHandle program) : program_(program) {}
  void Optimize() override;

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
  IRHandle program_;
};

}  // namespace polly