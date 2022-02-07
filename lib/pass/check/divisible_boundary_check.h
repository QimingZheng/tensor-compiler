/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:31:59
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-19 20:11:35
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "pass/pass.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

namespace polly {

/*!
 * \brief DivisibleBoundaryCheck checks whether the boundary of a loop can be
 * divided by a certain divisor. Please make sure that the boundary of it is
 * checked by the IRConstantBoundaryCheckVisitor checker first.
 *
 * \param loop The ir node (a ForNode) of the loop to be checked.
 * \param divisor The division factor.
 */
class DivisibleBoundaryCheck : public Pass, public IRNotImplementedVisitor {
 private:
  DivisibleBoundaryCheck() {}
  DivisibleBoundaryCheck(IRHandle loop, int divisor)
      : loop_(loop), divisor(divisor) {
    if (divisor > 0) {
      this->visit(loop_);
    } else {
      isDivisibleBoundary = false;
    }
    SetStatus(Pass::PassStatus::VALID);
  }

 public:
  bool isDivisibleBoundary;
  int divisor;
  int value = -1;

  static PassRetHandle runPass(PassArgHandle arg) {
    DivisibleBoundaryCheck checker(PassArg::as<Arg>(arg)->loop,
                                   PassArg::as<Arg>(arg)->divisor);
    auto ret = Ret::create(checker.isDivisibleBoundary);
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

  void visitMin(MinHandle min) override;
  void visitMax(MaxHandle max) override;

  struct Arg : public PassArg {
    IRHandle loop;
    int divisor;
    Arg() {}
    Arg(IRHandle loop_, int div) : loop(loop_), divisor(div) {}
  };

  struct Ret : public PassRet {
    bool isDivisibleBoundary;
    static PassRetHandle create(bool isDivisibleBoundary) {
      auto ret = std::shared_ptr<Ret>(new Ret);
      ret->isDivisibleBoundary = isDivisibleBoundary;
      return ret;
    }
  };

 private:
  IRHandle loop_;
};
}  // namespace polly