#pragma once

#include "common.h"
#include "transform_pass.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

namespace polly {

/// Before unrolling a loop, check its boundary is contant first.
class LoopUnroll : public TransformPass, public IRVisitor {
 public:
  LoopUnroll(IRHandle program, int unroll_limit = 8) : program_(program) {}

  void Transform() override { this->visit(program_); }

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
  IRHandle replaceVarWithInt(IRHandle node, IRHandle var, IRHandle int_expr);
  std::stack<IRHandle> tape_;
  IRHandle var, int_expr;
  IRHandle program_;
  bool reachInnerMostLoop = false;
};

}  // namespace polly