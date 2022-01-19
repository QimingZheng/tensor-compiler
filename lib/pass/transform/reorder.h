/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:30:44
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-19 20:37:44
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "pass/pass.h"

namespace polly {

class LoopReorder : public Pass, public IRSimpleVisitor {
 private:
  LoopReorder(IRHandle program, IRHandle i_loop, IRHandle j_loop);

 public:
  static PassRetHandle runPass(PassArgHandle arg);

  void visitFor(ForHandle loop) override;
  void visitFunc(FuncHandle func) override;

  struct Arg : public PassArg {
    IRHandle program;
    IRHandle i_loop;
    IRHandle j_loop;

    Arg() {}
    Arg(IRHandle program, IRHandle i_loop, IRHandle j_loop)
        : program(program), i_loop(i_loop), j_loop(j_loop) {}
    static PassArgHandle create(IRHandle p, IRHandle i, IRHandle j) {
      return std::shared_ptr<Arg>(new Arg(p, i, j));
    }
  };

  struct Ret : public PassRet {
    static PassRetHandle create() { return std::shared_ptr<Ret>(new Ret); }
  };

  IRHandle program_;
  IRHandle i_loop_;
  IRHandle j_loop_;

  std::vector<IRHandle> loop_vars_;

  bool modeling_;
  int ith_;
  int jth_;
};

}  // namespace polly
