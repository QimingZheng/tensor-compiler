/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-19 11:43:05
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-19 20:31:30
 * @CopyRight: Qiming Zheng
 */
#pragma once
#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "pass/pass.h"

namespace polly {

// Checks whether an expr node expresses a constant or not, does not accept
// statement as input IRNode.
class IsConstantIRHandle : public IRNotImplementedVisitor, public Pass {
 public:
  IsConstantIRHandle(IRHandle handle)
      : IRNotImplementedVisitor("should not be visited") {
    isConstant = false;
    handle.accept(this);
  }

  static PassRetHandle runPass(PassArgHandle arg) {
    IsConstantIRHandle checker(PassArg::as<Arg>(arg)->handle);
    return Ret::create(checker.isConstant);
  }

  void visitInt(IntHandle int_expr) override { isConstant = true; }
  void visitAdd(AddHandle add) override {
    add->lhs.accept(this);
    if (!isConstant) return;
    add->rhs.accept(this);
  }
  void visitSub(SubHandle sub) override {
    sub->lhs.accept(this);
    if (!isConstant) return;
    sub->rhs.accept(this);
  }
  void visitMul(MulHandle mul) override {
    mul->lhs.accept(this);
    if (!isConstant) return;
    mul->rhs.accept(this);
  }
  void visitDiv(DivHandle div) override {
    div->lhs.accept(this);
    if (!isConstant) return;
    div->rhs.accept(this);
  }
  void visitMod(ModHandle mod) override {
    mod->lhs.accept(this);
    if (!isConstant) return;
    mod->rhs.accept(this);
  }
  void visitVar(VarHandle var) override { isConstant = false; }
  void visitAccess(AccessHandle access) override { isConstant = false; }
  void visitTensor(TensorHandle tensor) override { isConstant = false; }

  struct Arg : public PassArg {
    IRHandle handle;
    Arg() {}
    Arg(IRHandle handle) : handle(handle) {}
    Arg(std::shared_ptr<IRNode> node) : handle(IRHandle(node)) {}
  };
  struct Ret : public PassRet {
    bool isConstant;
    static PassRetHandle create(bool isConstant) {
      auto ret = std::shared_ptr<Ret>(new Ret);
      ret->isConstant = isConstant;
      return ret;
    }
  };
  bool isConstant;
};

}  // namespace polly
