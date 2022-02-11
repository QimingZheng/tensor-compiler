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
// 1. Vectorization Pass should always be the last step.
// 2. The Loop been vectorized should always be of the form:
// 0 -> a divisible boundary by vecLen, (1)
class LoopVectorization : public Pass, public IRNotImplementedVisitor {
 public:
  constexpr static PassKey id = LoopVectorizationPassID;
  LoopVectorization(IRHandle program, IRHandle loop, int vecLen)
      : program_(program), loop_(loop), vecLen(vecLen) {}

  static PassRetHandle runPass(PassArgHandle arg) {
    LoopVectorization vec(PassArg::as<Arg>(arg)->program,
                          PassArg::as<Arg>(arg)->loop,
                          PassArg::as<Arg>(arg)->vecLen);
    vec.visit(vec.loop_);
    return Ret::create();
  }

  void visitInt(IntHandle int_expr) override;
  void visitFloat(FloatHandle float_expr) override;
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

  void visitMin(MinHandle min) override;
  void visitMax(MaxHandle max) override;

  struct Arg : public PassArg {
    IRHandle program;
    IRHandle loop;
    int vecLen;
    Arg() {}
    Arg(IRHandle p, IRHandle l, int v) : program(p), loop(l), vecLen(v) {}
    static PassArgHandle create(IRHandle p, IRHandle l, int v) {
      return std::shared_ptr<Arg>(new Arg(p, l, v));
    }
  };
  struct Ret : public PassRet {
    static PassRetHandle create() { return std::shared_ptr<Ret>(new Ret); }
  };

  IRHandle program_;
  IRHandle loop_;
  int vecLen;

  std::vector<IRHandle> vectorizationBody;
  IRHandle node;
};

}  // namespace polly
