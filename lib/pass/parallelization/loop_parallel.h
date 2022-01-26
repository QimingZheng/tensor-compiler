/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-23 19:17:06
 * @Last Modified by:   Qiming Zheng
 * @Last Modified time: 2022-01-23 19:17:06
 * @CopyRight: Qiming Zheng
 */

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "pass/pass.h"
#include "pass/analysis/parallelization_analysis_pass.h"

namespace polly {

/// Simply parallelize every parallelizable loop.
class LoopParallel : public Pass, public IRNotImplementedVisitor {
  LoopParallel(IRHandle program) : program_(program) { program_.accept(this); }

 public:
  void visitFor(ForHandle loop) override;
  void visitFunc(FuncHandle func) override;

  static PassRetHandle runPass(PassArgHandle arg) {
    LoopParallel(PassArg::as<Arg>(arg)->program);
    return Ret::create();
  }

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

  IRHandle program_;
};

}  // namespace polly