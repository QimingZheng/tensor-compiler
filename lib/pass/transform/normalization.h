/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:31:00
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-19 20:27:59
 * @CopyRight: Qiming Zheng
 */
#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "pass/pass.h"

namespace polly {

/// Normalization Pass will transform an affine program into standarized form.
/// Including:
///     1. Make the incremental amount always equal to 1.
class NormalizationPass : public Pass, public IRVisitor {
 private:
  NormalizationPass(IRHandle p) : program_(p) { program_.accept(this); }

 public:
  static PassRetHandle runPass(PassArgHandle arg);

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

  bool isNormalized(IRHandle var) {
    IRHandle min = var.as<VarNode>()->min;
    IRHandle max = var.as<VarNode>()->max;
    IRHandle increment = var.as<VarNode>()->increment;
    if (increment.Type() != IRNodeType::INT) {
      return false;
    } else if (increment.as<IntNode>()->value != 1) {
      return false;
    }
    return true;
  }

  struct Arg : public PassArg {
    IRHandle program;
    Arg() {}
    Arg(IRHandle p) : program(p) {}
  };

  struct Ret : public PassRet {
    static PassRetHandle create() { return std::shared_ptr<Ret>(new Ret); }
  };

  IRHandle program_;
};

}  // namespace polly
