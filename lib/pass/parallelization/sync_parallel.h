/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:31:28
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-28 15:54:52
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "pass/pass.h"
#include "pass/parallelization/parallel_utils.h"
#include "pass/analysis/polyhedral_extraction.h"
#include "pass/analysis/data_dependency_model.h"

namespace polly {

// Find Strongly Connected Components in a program, then break the program
// according to each statement's connection component (using Fission/Fussion).
class SyncParallel : public IRNotImplementedVisitor, public Pass {
  SyncParallel(IRHandle program) : program_(program) { program_.accept(this); }

 public:
  static PassRetHandle runPass(PassArgHandle arg);

  void visitFor(ForHandle loop) override;
  void visitFunc(FuncHandle func) override;

  struct Arg : public PassArg {
    IRHandle program;
    Arg() {}
    Arg(IRHandle p) : program(p) {}
    static PassArgHandle create(IRHandle p) {
      return std::shared_ptr<Arg>(new Arg(p));
    }
  };

  struct Ret : public PassRet {
    static PassRetHandle create() { return std::shared_ptr<Ret>(new Ret); }
  };

  static std::vector<IRHandle> Adjust(
      std::vector<IRHandle> enclosing_looping_vars, IRHandle loop);

  std::vector<IRHandle> enclosing_looping_vars_;
  IRHandle program_;
};

}  // namespace polly