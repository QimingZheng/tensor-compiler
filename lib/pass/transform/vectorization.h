/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:30:29
 * @Last Modified by:   Qiming Zheng
 * @Last Modified time: 2022-01-18 20:30:29
 * @CopyRight: Qiming Zheng
 */
#pragma once

#include "common.h"
#include "pass/pass.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "ir/ir_module.h"

namespace polly {

// Vectorize an inner most loop.
class LoopVectorization : public Pass, public IRVisitor {
 public:
  constexpr static PassKey id = LoopVectorizationPassID;
  LoopVectorization(IRHandle program, IRHandle loop)
      : program_(program), loop_(loop) {}

  static PassRetHandle runPass(PassArgHandle arg) {}

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

  struct Arg : public PassArg {};
  struct Ret : public PassRet {};

  IRHandle program_;
  IRHandle loop_;
};

}  // namespace polly
