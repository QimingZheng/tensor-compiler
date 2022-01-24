#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "pass/pass.h"
#include "polyhedral_extraction.h"
#include "polyhedral_model.h"
#include "data_dependency_model.h"

namespace polly {

// Check if there are two different instances of a loop that exist data
// dependency.
class ParallelizationAnalysisPass : public Pass, public IRRecursiveVisitor {
  ParallelizationAnalysisPass(IRHandle program, IRHandle loop)
      : program_(program), loop_(loop) {
    nesting_loop_vars_.clear();
    found_ = false;
    program_.accept(this);
  }

 public:
  static PassRetHandle runPass(PassArgHandle arg);

  void visitFor(ForHandle loop) override;
  void visitFunc(FuncHandle func) override;

  struct Arg : public PassArg {
    IRHandle program;
    IRHandle loop;
    Arg() {}
    Arg(IRHandle program, IRHandle loop) : program(program), loop(loop) {}
    static PassArgHandle create(IRHandle program, IRHandle loop) {
      return std::shared_ptr<Arg>(new Arg(program, loop));
    }
  };

  struct Ret : public PassRet {
    bool legal;
    Ret() {}
    Ret(bool x) : legal(x) {}
    static PassRetHandle create(bool x) {
      return std::shared_ptr<Ret>(new Ret(x));
    }
  };

  bool found_;
  std::vector<int> prog_context_;
  std::vector<IRHandle> nesting_loop_vars_;

  IRHandle program_;
  IRHandle loop_;
};

}  // namespace polly