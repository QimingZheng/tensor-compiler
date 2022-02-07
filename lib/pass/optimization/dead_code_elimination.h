/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:31:39
 * @Last Modified by:   Qiming Zheng
 * @Last Modified time: 2022-01-18 20:31:39
 * @CopyRight: Qiming Zheng
 */
#pragma once

#include "common.h"
#include "ir/ir_visitor.h"
#include "pass/analysis/polyhedral_extraction.h"
#include "pass/pass.h"

namespace polly {

// Dead code elimination removes empty loops and its nesting childrens.
class DeadCodeElimination : public Pass, public IRNotImplementedVisitor {
  DeadCodeElimination(IRHandle program) : program_(program) {
    program_.accept(this);
  }

 public:
  static PassRetHandle runPass(PassArgHandle arg)   {
    DeadCodeElimination eliminator(PassArg::as<Arg>(arg)->program);
    return Ret::create();
  }

  void visitFor(ForHandle loop) override;
  void visitFunc(FuncHandle func) override;

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
  std::vector<IRHandle> enclosing_looping_vars_;
};

}  // namespace polly