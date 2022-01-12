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