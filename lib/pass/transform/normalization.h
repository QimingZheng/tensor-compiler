/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:31:00
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-26 11:41:34
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
class NormalizationPass : public Pass, public IRNotImplementedVisitor {
 private:
  NormalizationPass(IRHandle p) : program_(p) { program_.accept(this); }

 public:
  static PassRetHandle runPass(PassArgHandle arg);

  void visitFor(ForHandle loop) override;
  void visitFunc(FuncHandle func) override;

  bool isNormalized(IRHandle var) {
    assert(var.Type() == IRNodeType::VAR);

    IRHandle min = var.as<VarNode>()->min;
    IRHandle max = var.as<VarNode>()->max;
    IRHandle increment = var.as<VarNode>()->increment;

    if (min != IntNode::make(0)) {
      return false;
    }

    if (increment != IntNode::make(1)) {
      return false;
    }
    return true;
  }

  struct Arg : public PassArg {
    IRHandle program;
    Arg() {}
    Arg(IRHandle p) : program(p) {}
    static PassArgHandle create(IRHandle program) {
      return std::shared_ptr<Arg>(new Arg(program));
    }
  };

  struct Ret : public PassRet {
    static PassRetHandle create() { return std::shared_ptr<Ret>(new Ret); }
  };

  IRHandle program_;
};

}  // namespace polly
