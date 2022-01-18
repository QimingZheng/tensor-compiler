/*
 * @Description: Polly: A DSL compiler for Tensor Program 
 * @Author: Qiming Zheng 
 * @Date: 2022-01-18 20:31:59 
 * @Last Modified by:   Qiming Zheng 
 * @Last Modified time: 2022-01-18 20:31:59 
 * @CopyRight: Qiming Zheng 
 */

#pragma once

#include "common.h"
#include "pass/pass.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

namespace polly {

/// DivisibleBoundaryCheck checks whether the boundary of a loop can be
/// divided by a certain divisor. Please make sure that the boundary of it is
/// checked by the IRConstantBoundaryCheckVisitor checker first.
class DivisibleBoundaryCheck : public Pass, public IRVisitor {
 public:
  bool isDivisibleBoundary;
  int divisor;
  int value = -1;
  DivisibleBoundaryCheck() {}
  DivisibleBoundaryCheck(IRHandle loop, int divisor)
      : loop_(loop), divisor(divisor) {}

  PassRetHandle runPass(PassArgHandle arg) override {
    loop_ = PassArg::as<Arg>(arg)->loop;
    divisor = PassArg::as<Arg>(arg)->divisor;
    this->visit(loop_);
    SetStatus(Pass::PassStatus::VALID);
    auto ret = std::shared_ptr<Ret>(new Ret);
    ret->isDivisibleBoundary = isDivisibleBoundary;
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
    IRHandle loop;
    int divisor;
    Arg() {}
    Arg(IRHandle loop_, int div) : loop(loop_), divisor(div) {}
  };

  struct Ret : public PassRet {
    bool isDivisibleBoundary;
  };

 private:
  IRHandle loop_;
};
}  // namespace polly