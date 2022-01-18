/*
 * @Description: Polly: A DSL compiler for Tensor Program 
 * @Author: Qiming Zheng 
 * @Date: 2022-01-18 20:31:03 
 * @Last Modified by:   Qiming Zheng 
 * @Last Modified time: 2022-01-18 20:31:03 
 * @CopyRight: Qiming Zheng 
 */
#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "pass/pass.h"

namespace polly {

class FussionTransform : public Pass, public IRVisitor {
 public:
  constexpr static PassKey id = FussionPassID;

  FussionTransform() {}
  FussionTransform(IRHandle program, IRHandle first, IRHandle second)
      : program_(program), firstLoop(first), secondLoop(second) {
    searching_ = true;
  }

  PassRetHandle runPass(PassArgHandle arg) override;

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

  bool findLoop(std::vector<IRHandle>& handles, IRHandle target);
  IRHandle replace_if_match(IRHandle origin);

  IRHandle firstLoop;
  IRHandle secondLoop;
  IRHandle program_;

  bool searching_;
  struct Arg : public PassArg {
    IRHandle program;
    IRHandle firstLoop;
    IRHandle secondLoop;
    Arg() {}
    Arg(IRHandle program, IRHandle firstLoop, IRHandle secondLoop)
        : program(program), firstLoop(firstLoop), secondLoop(secondLoop) {}
  };
  struct Ret : public PassRet {};
};

}  // namespace polly