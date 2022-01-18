/*
 * @Description: Polly: A DSL compiler for Tensor Program 
 * @Author: Qiming Zheng 
 * @Date: 2022-01-18 20:30:39 
 * @Last Modified by:   Qiming Zheng 
 * @Last Modified time: 2022-01-18 20:30:39 
 * @CopyRight: Qiming Zheng 
 */

#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "pass/pass.h"

namespace polly {

/// It can be proved that a split operation does not change the semantics of a
/// program.
class LoopSplit : public Pass, public IRVisitor {
 public:
  LoopSplit() { searching_ = true; }
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

  IRHandle replace_with(IRHandle node);

  struct Arg : public PassArg {
    IRHandle program;
    IRHandle loop;
    int splitFactor;
    Arg() {}
    Arg(IRHandle p, IRHandle l, int s) : program(p), loop(l), splitFactor(s) {}
  };

  struct Ret : public PassRet {};

  IRHandle program_;
  IRHandle loop_;
  IRHandle replace_;
  int splitFactor;

  bool searching_;
};

}  // namespace polly