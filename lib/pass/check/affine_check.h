/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:31:50
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-19 20:22:17
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "pass/pass.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

#include "constant_expr_check.h"
#include "affine_expr_check.h"

namespace polly {

// AffineCheck checks whether a program is an affine program or not.
// To be an legal affine program, all the loop bounds shoud be expressed as
// affine expressions and the incremental amount should be constant number.
// Array access expression in statements should be affine also.
class AffineCheck : public IRNotImplementedVisitor, public Pass {
 private:
  AffineCheck() {}

  AffineCheck(IRHandle program) : program_(program), firstTimeEntering(false) {
    this->visit(program_);
    SetStatus(Pass::PassStatus::VALID);
  }

 public:
  bool isAffine;

  bool firstTimeEntering;

  static PassRetHandle runPass(PassArgHandle arg) {
    AffineCheck checker(PassArg::as<Arg>(arg)->program);
    return Ret::create(checker.isAffine);
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
    bool isAffine;
    static PassRetHandle create(bool isAffine) {
      auto ret = std::shared_ptr<Ret>(new Ret);
      ret->isAffine = isAffine;
      return ret;
    }
  };

 private:
  IRHandle program_;
};

}  // namespace polly