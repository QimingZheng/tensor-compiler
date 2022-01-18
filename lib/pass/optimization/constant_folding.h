/*
 * @Description: Polly: A DSL compiler for Tensor Program 
 * @Author: Qiming Zheng 
 * @Date: 2022-01-18 20:29:26 
 * @Last Modified by:   Qiming Zheng 
 * @Last Modified time: 2022-01-18 20:29:26 
 * @CopyRight: Qiming Zheng 
 */

#pragma once

#include "common.h"
#include "pass/pass.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

namespace polly {

class ConstantFoldingPass : public Pass, public IRVisitor {
 public:
  constexpr static PassKey id = ConstantFoldingPassID;

  ConstantFoldingPass() {}
  ConstantFoldingPass(IRHandle program) : program_(program) {}

  PassRetHandle runPass(PassArgHandle arg) override {
    program_ = PassArg::as<Arg>(arg)->program;

    this->program_.accept(this);
    SetStatus(Pass::PassStatus::VALID);
    auto ret = std::shared_ptr<Ret>(new Ret);
    return ret;
  }

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

  struct Arg : public PassArg {
    IRHandle program;
    Arg() {}
    Arg(IRHandle p) : program(p) {}
  };

  struct Ret : public PassRet {};

 private:
  IRHandle program_;
};

}  // namespace polly