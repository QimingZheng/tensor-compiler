/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-19 12:22:02
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-19 20:32:25
 * @CopyRight: Qiming Zheng
 */
#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "pass/pass.h"

#include "./constant_expr_check.h"

namespace polly {

// Checks whether an expr node represents an affine expression or not, does not
// accept statement as input IRNode.
class IsAffineIRHandle : public IRNotImplementedVisitor, public Pass {
 public:
  IsAffineIRHandle(IRHandle handle)
      : IRNotImplementedVisitor("should not be visited") {
    isAffine = false;
    handle.accept(this);
  }

  static PassRetHandle runPass(PassArgHandle arg) {
    IsAffineIRHandle checker(PassArg::as<Arg>(arg)->handle);
    return Ret::create(checker.isAffine);
  }

  void visitInt(IntHandle int_expr) override { isAffine = true; }
  void visitAdd(AddHandle add) override {
    add->lhs.accept(this);
    if (!isAffine) return;
    add->rhs.accept(this);
    if (!isAffine) return;
  }
  void visitSub(SubHandle sub) override {
    sub->lhs.accept(this);
    if (!isAffine) return;
    sub->rhs.accept(this);
    if (!isAffine) return;
  }
  void visitMul(MulHandle mul) override {
    mul->lhs.accept(this);
    if (!isAffine) return;
    mul->rhs.accept(this);
    if (!isAffine) return;
    auto l =
        IsConstantIRHandle::runPass(std::shared_ptr<IsConstantIRHandle::Arg>(
            new IsConstantIRHandle::Arg(mul->lhs)));
    auto r =
        IsConstantIRHandle::runPass(std::shared_ptr<IsConstantIRHandle::Arg>(
            new IsConstantIRHandle::Arg(mul->rhs)));
    if (!PassRet::as<IsConstantIRHandle::Ret>(l)->isConstant &&
        !PassRet::as<IsConstantIRHandle::Ret>(r)->isConstant) {
      isAffine = false;
      return;
    }
  }
  void visitDiv(DivHandle div) override {
    div->lhs.accept(this);
    if (!isAffine) return;
    auto r =
        IsConstantIRHandle::runPass(std::shared_ptr<IsConstantIRHandle::Arg>(
            new IsConstantIRHandle::Arg(div->rhs)));
    if (!PassRet::as<IsConstantIRHandle::Ret>(r)->isConstant) {
      isAffine = false;
      return;
    }
  }
  void visitMod(ModHandle mod) override {
    mod->lhs.accept(this);
    if (!isAffine) return;
    auto r =
        IsConstantIRHandle::runPass(std::shared_ptr<IsConstantIRHandle::Arg>(
            new IsConstantIRHandle::Arg(mod->rhs)));
    if (!PassRet::as<IsConstantIRHandle::Ret>(r)->isConstant) {
      isAffine = false;
      return;
    }
  }
  void visitVar(VarHandle var) override { isAffine = true; }
  void visitAccess(AccessHandle access) override { isAffine = false; }
  void visitTensor(TensorHandle tensor) override { isAffine = false; }

  struct Arg : public PassArg {
    IRHandle handle;
    Arg() {}
    Arg(IRHandle handle) : handle(handle) {}
    Arg(std::shared_ptr<IRNode> node) : handle(IRHandle(node)) {}
  };
  struct Ret : public PassRet {
    bool isAffine;
    static PassRetHandle create(bool isAffine) {
      auto ret = std::shared_ptr<Ret>(new Ret);
      ret->isAffine = isAffine;
      return ret;
    }
  };
  bool isAffine;
};
}  // namespace polly