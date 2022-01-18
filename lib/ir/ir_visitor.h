/*
 * @Description: Polly: A DSL compiler for Tensor Program 
 * @Author: Qiming Zheng 
 * @Date: 2022-01-18 20:32:45 
 * @Last Modified by:   Qiming Zheng 
 * @Last Modified time: 2022-01-18 20:32:45 
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
  void visitInt(IntHandle int_expr) override { /* Pass */ }
    void visitAdd(AddHandle add) override { /* Pass */ }
    void visitSub(SubHandle sub) override { /* Pass */ }
    void visitMul(MulHandle mul) override { /* Pass */ }
    void visitDiv(DivHandle div) override { /* Pass */ }
    void visitMod(ModHandle mod) override { /* Pass */ }
    void visitVar(VarHandle var) override { /* Pass */ }
    void visitAccess(AccessHandle access) override { /* Pass */ }
    void visitAssign(AssignmentHandle assign) override { /* Pass */ }
    void visitTensor(TensorHandle tensor) override { /* Pass */ }
    void visitFor(ForHandle loop) override { /* Pass */ }
    void visitConst(ConstHandle con) override { /* Pass */ }
    void visitPrint(PrintHandle print) override { /* Pass */ }
    void visitFunc(FuncHandle func) override { /* Pass */ }
};

class IRNotImplementedVisitor : public IRVisitor {
 public:
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
  void throw_exception(std::string msg) {
    throw std::runtime_error(msg + "Node visit method Not Implemented");
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