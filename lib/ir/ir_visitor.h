/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:32:45
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-19 16:03:43
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "ir.h"

namespace polly {

class IRVisitor {
 public:
  virtual void visit(IRHandle expr);
  virtual void visitInt(IntHandle int_expr) = 0;
  virtual void visitAdd(AddHandle add) = 0;
  virtual void visitSub(SubHandle sub) = 0;
  virtual void visitMul(MulHandle mul) = 0;
  virtual void visitDiv(DivHandle div) = 0;
  virtual void visitMod(ModHandle mod) = 0;
  virtual void visitVar(VarHandle var) = 0;
  virtual void visitAssign(AssignmentHandle assign) = 0;
  virtual void visitAccess(AccessHandle access) = 0;
  virtual void visitTensor(TensorHandle tensor) = 0;
  virtual void visitFor(ForHandle loop) = 0;
  virtual void visitConst(ConstHandle con) = 0;
  virtual void visitPrint(PrintHandle print) = 0;
  virtual void visitFunc(FuncHandle func) = 0;
};

class IRSimpleVisitor : public IRVisitor {
 public:
  void visitInt(IntHandle int_expr) override { helper(IRHandle(int_expr)); }
  void visitAdd(AddHandle add) override { helper(IRHandle(add)); }
  void visitSub(SubHandle sub) override { helper(IRHandle(sub)); }
  void visitMul(MulHandle mul) override { helper(IRHandle(mul)); }
  void visitDiv(DivHandle div) override { helper(IRHandle(div)); }
  void visitMod(ModHandle mod) override { helper(IRHandle(mod)); }
  void visitVar(VarHandle var) override { helper(IRHandle(var)); }
  void visitAccess(AccessHandle access) override { helper(IRHandle(access)); }
  void visitAssign(AssignmentHandle assign) override {
    helper(IRHandle(assign));
  }
  void visitTensor(TensorHandle tensor) override { helper(IRHandle(tensor)); }
  void visitFor(ForHandle loop) override { helper(IRHandle(loop)); }
  void visitConst(ConstHandle con) override { helper(IRHandle(con)); }
  void visitPrint(PrintHandle print) override { helper(IRHandle(print)); }
  void visitFunc(FuncHandle func) override { helper(IRHandle(func)); }

  virtual void helper(IRHandle node) { return; }
};

class IRRecursiveVisitor : public IRVisitor {
 public:
  void visitInt(IntHandle int_expr) override {
    enter(IRHandle(int_expr));
    exit(IRHandle(int_expr));
  }
  void visitAdd(AddHandle add) override {
    enter(IRHandle(add));
    add->lhs.accept(this);
    add->rhs.accept(this);
    exit(IRHandle(add));
  }
  void visitSub(SubHandle sub) override {
    enter(IRHandle(sub));
    sub->lhs.accept(this);
    sub->rhs.accept(this);
    exit(IRHandle(sub));
  }
  void visitMul(MulHandle mul) override {
    enter(IRHandle(mul));
    mul->lhs.accept(this);
    mul->rhs.accept(this);
    exit(IRHandle(mul));
  }
  void visitDiv(DivHandle div) override {
    enter(IRHandle(div));
    div->lhs.accept(this);
    div->rhs.accept(this);
    exit(IRHandle(div));
  }
  void visitMod(ModHandle mod) override {
    enter(IRHandle(mod));
    mod->lhs.accept(this);
    mod->rhs.accept(this);
    exit(IRHandle(mod));
  }
  void visitVar(VarHandle var) override {
    enter(IRHandle(var));
    var->min.accept(this);
    var->max.accept(this);
    var->increment.accept(this);
    exit(IRHandle(var));
  }
  void visitAccess(AccessHandle access) override {
    enter(IRHandle(access));
    access->tensor.accept(this);
    for (int i = 0; i < access->indices.size(); i++)
      access->indices[i].accept(this);
    exit(IRHandle(access));
  }
  void visitAssign(AssignmentHandle assign) override {
    enter(IRHandle(assign));
    assign->lhs.accept(this);
    assign->rhs.accept(this);
    exit(IRHandle(assign));
  }
  void visitTensor(TensorHandle tensor) override {
    enter(IRHandle(tensor));
    exit(IRHandle(tensor));
  }
  void visitFor(ForHandle loop) override {
    enter(IRHandle(loop));
    loop->looping_var_.accept(this);
    for (int i = 0; i < loop->body.size(); i++) loop->body[i].accept(this);
    exit(IRHandle(loop));
  }
  void visitConst(ConstHandle con) override {
    enter(IRHandle(con));
    exit(IRHandle(con));
  }
  void visitPrint(PrintHandle print) override {
    enter(IRHandle(print));
    print->print.accept(this);
    exit(IRHandle(print));
  }
  void visitFunc(FuncHandle func) override {
    enter(IRHandle(func));
    for (int i = 0; i < func->body.size(); i++) func->body[i].accept(this);
    exit(IRHandle(func));
  }

  virtual void enter(IRHandle node) { return; }
  virtual void exit(IRHandle node) { return; }
};

class IRNotImplementedVisitor : public IRVisitor {
 public:
  IRNotImplementedVisitor(std::string errorMsg = "visit method Not Implemented")
      : errorMsg(errorMsg) {}
  void visitInt(IntHandle int_expr) override { throw_exception("Int"); }
  void visitAdd(AddHandle add) override { throw_exception("Add"); }
  void visitSub(SubHandle sub) override { throw_exception("Sub"); }
  void visitMul(MulHandle mul) override { throw_exception("Mul"); }
  void visitDiv(DivHandle div) override { throw_exception("Div"); }
  void visitMod(ModHandle mod) override { throw_exception("Mod"); }
  void visitVar(VarHandle var) override { throw_exception("Var"); }
  void visitAccess(AccessHandle access) override { throw_exception("Access"); }
  void visitAssign(AssignmentHandle assign) override {
    throw_exception("Assignment");
  }
  void visitTensor(TensorHandle tensor) override { throw_exception("Tensor"); }
  void visitFor(ForHandle loop) override { throw_exception("Loop"); }
  void visitConst(ConstHandle con) override { throw_exception("Constant"); }
  void visitPrint(PrintHandle print) override { throw_exception("Print"); }
  void visitFunc(FuncHandle func) override { throw_exception("Func"); }

 private:
  std::string errorMsg;
  void throw_exception(std::string msg) {
    throw std::runtime_error(msg + "Node " + errorMsg);
  }
};

class IRPrinterVisitor : public IRVisitor {
 public:
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
};

class IRMutatorVisitor : public IRVisitor {
 public:
  IRHandle replaceFrom;
  IRHandle replaceTo;
  IRMutatorVisitor(IRHandle replaceFrom, IRHandle replaceTo)
      : replaceFrom(replaceFrom), replaceTo(replaceTo) {}

  template <typename T>
  T _replace_subnode_helper(T op_node, bool skipReplace = false);

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
};

}  // namespace polly