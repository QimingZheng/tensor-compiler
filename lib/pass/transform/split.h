/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:30:39
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-19 18:26:13
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "pass/pass.h"

namespace polly {

/// It can be proved that a split operation does not change the semantics of a
/// program.
// Make sure that the splitting transformation is done after the normalization
// pass.
class LoopSplit : public Pass, public IRNotImplementedVisitor {
 private:
  LoopSplit(IRHandle p, IRHandle l, int s)
      : program_(p), loop_(l), splitFactor(s) {
    searching_ = true;
    program_.accept(this);
  }

 public:
  static PassRetHandle runPass(PassArgHandle arg);

  void visitInt(IntHandle int_expr) override;
  void visitFloat(FloatHandle float_expr) override;
  void visitAdd(AddHandle add) override;
  void visitSub(SubHandle sub) override;
  void visitMul(MulHandle mul) override;
  void visitDiv(DivHandle div) override;
  void visitMod(ModHandle mod) override;
  void visitVar(VarHandle var) override;
  void visitAccess(AccessHandle access) override;
  void visitAssign(AssignmentHandle assign) override;
  void visitTensor(TensorHandle tensor) override;
  void visitVal(ValHandle val) override;
  void visitDecl(DeclHandle decl) override;
  void visitFor(ForHandle loop) override;
  void visitConst(ConstHandle con) override;
  void visitPrint(PrintHandle print) override;
  void visitFunc(FuncHandle func) override;

  void visitMin(MinHandle min) override;
  void visitMax(MaxHandle max) override;

  IRHandle replace_with(IRHandle node);

  IRHandle get_outter_loop_var(IRHandle loop_var);
  IRHandle get_inner_loop_var();
  IRHandle get_remainder_loop_var(IRHandle loop_var);

  IRHandle create_remainder_loop(IRHandle loop, IRHandle remainder_loop_var);

  struct Arg : public PassArg {
    IRHandle program;
    IRHandle loop;
    int splitFactor;
    Arg() {}
    Arg(IRHandle p, IRHandle l, int s) : program(p), loop(l), splitFactor(s) {}
    static PassArgHandle create(IRHandle p, IRHandle l, int s) {
      return std::shared_ptr<Arg>(new Arg(p, l, s));
    }
  };

  struct Ret : public PassRet {
    static PassRetHandle create() { return std::shared_ptr<Ret>(new Ret); }
  };

  IRHandle program_;
  IRHandle loop_;
  IRHandle replace_;

  std::vector<IRHandle> enclosing_looping_vars_;
  std::map<IRNodeKey, IRHandle> outter_dict;

  IRHandle outter_loop_var_, inner_loop_var_;
  int splitFactor;

  bool searching_;
};

}  // namespace polly