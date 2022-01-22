/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:31:55
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-19 20:13:50
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "pass/pass.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

namespace polly {

// A ConstantBoundary is a for loop bounded by [0, C) C is an integer constant,
// and the incremental amount must be one.
// A loop with constatnt boundary has many good properties, e.g. it can be
// unrolled/vectorized etc.
class ConstantBoundaryCheck : public Pass, public IRNotImplementedVisitor {
 private:
  ConstantBoundaryCheck() {}

  ConstantBoundaryCheck(IRHandle program) : program_(program) {
    this->visit(program_);
    SetStatus(Pass::PassStatus::VALID);
  }

 public:
  bool isConstantBoundary;
  int value = -1;

  static PassRetHandle runPass(PassArgHandle arg) {
    ConstantBoundaryCheck checker(PassArg::as<Arg>(arg)->program);
    return Ret::create(checker.isConstantBoundary);
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
  };

  struct Ret : public PassRet {
    bool isConstantBoundary;
    static PassRetHandle create(bool isConstantBoundary) {
      auto ret = std::shared_ptr<Ret>(new Ret);
      ret->isConstantBoundary = isConstantBoundary;
      return ret;
    }
  };

 private:
  IRHandle program_;
};

}  // namespace polly