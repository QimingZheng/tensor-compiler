/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:31:06
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-19 20:09:51
 * @CopyRight: Qiming Zheng
 */
#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "pass/pass.h"

namespace polly {

class FissionTransform : public Pass, public IRNotImplementedVisitor {
 private:
  FissionTransform() { searching_ = true; }

  FissionTransform(IRHandle program, IRHandle targetLoop)
      : program_(program), loop_(targetLoop) {
    searching_ = true;
    program_.accept(this);
  }

 public:
  constexpr static PassKey id = FissionPassID;

  static PassRetHandle runPass(PassArgHandle arg) {
    FissionTransform(PassArg::as<Arg>(arg)->program,
                     PassArg::as<Arg>(arg)->targetLoop);

    return Ret::create();
  }

  IRHandle replace_if_match(IRHandle origin);

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

  IRHandle program_;
  IRHandle loop_;
  IRHandle replace_loop_;
  bool searching_;

  struct Arg : public PassArg {
    IRHandle program;
    IRHandle targetLoop;
    Arg() {}
    Arg(IRHandle program, IRHandle targetLoop)
        : program(program), targetLoop(targetLoop) {}
    static PassArgHandle create(IRHandle program, IRHandle targetLoop) {
      return std::shared_ptr<Arg>(new Arg(program, targetLoop));
    }
  };

  struct Ret : public PassRet {
    static PassRetHandle create() { return std::shared_ptr<Ret>(new Ret); }
  };
};

}  // namespace polly
