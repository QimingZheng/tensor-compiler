/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:30:32
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-19 19:50:27
 * @CopyRight: Qiming Zheng
 */
#pragma once

#include "common.h"
#include "pass/pass.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

namespace polly {

/// Before unrolling a loop, check its boundary is contant first.
class LoopUnroll : public Pass, public IRNotImplementedVisitor {
 private:
  LoopUnroll() {}

  LoopUnroll(IRHandle program) : program_(program) {}

 public:
  constexpr static PassKey id = UnrollPassID;

  static PassRetHandle runPass(PassArgHandle arg) {
    LoopUnroll unroll(PassArg::as<Arg>(arg)->program);
    unroll.visit(unroll.program_);
    auto ret = Ret::create();
    return ret;
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

  struct Arg : public PassArg {
    IRHandle program;
    Arg() {}
    Arg(IRHandle p) : program(p) {}
    static PassArgHandle create(IRHandle p) {
      return std::shared_ptr<Arg>(new Arg(p));
    }
  };
  struct Ret : public PassRet {
    static PassRetHandle create() { return std::shared_ptr<Ret>(new Ret); }
  };

 private:
  IRHandle replaceVarWithInt(IRHandle node, IRHandle var, IRHandle int_expr);
  std::stack<IRHandle> tape_;
  IRHandle var, int_expr;
  IRHandle program_;
  bool reachInnerMostLoop = false;
};

}  // namespace polly