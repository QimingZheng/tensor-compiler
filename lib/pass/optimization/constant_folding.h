/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:29:26
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-19 20:10:21
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "pass/pass.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

namespace polly {

// Constant Folding is a common compiler optimization pass that compute know
// algebra results ahead of executions.
class ConstantFoldingPass : public Pass, public IRVisitor {
 private:
  ConstantFoldingPass() {}
  ConstantFoldingPass(IRHandle program) : program_(program) {
    folded = true;
    while (folded) {
      this->program_.accept(this);
      folded = false;
    }
  }

 public:
  constexpr static PassKey id = ConstantFoldingPassID;

  static PassRetHandle runPass(PassArgHandle arg) {
    ConstantFoldingPass(PassArg::as<Arg>(arg)->program);
    return Ret::create();
  }

  // Simplify the calculation of a single node. Including:
  // 1. 0 + x = x; x + 0 = x;
  // 2. x - 0 = x; (TODO: 0 - x = -x;)
  // 3: x * 1 = x; 1 * x = x; 0 * x = 0; x * 0 = 0;
  // 4: x / 1 = x;
  // 5. x % 1 = 0;
  IRHandle simplify(IRHandle node);

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
  };

  struct Ret : public PassRet {
    static PassRetHandle create() { return std::shared_ptr<Ret>(new Ret); }
  };

 private:
  IRHandle program_;
  bool folded;
};

}  // namespace polly