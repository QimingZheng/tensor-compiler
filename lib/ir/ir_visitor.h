#pragma once

#include "common.h"
#include "ir.h"

namespace polly {

class IRVisitor {
 public:
  virtual void visit(IRNode *expr);
  virtual void visitInt(IntNode *int_expr) = 0;
  virtual void visitAdd(AddNode *add) = 0;
  virtual void visitMul(MulNode *mul) = 0;
  virtual void visitVar(VarNode *var) = 0;
  virtual void visitAssign(AssignmentNode *assign) = 0;
  virtual void visitAccess(AccessNode *access) = 0;
  virtual void visitTensor(TensorNode *tensor) = 0;
  virtual void visitFor(ForNode *loop) = 0;
};

class IRPrinterVisitor : public IRVisitor {
 public:
  void visitInt(IntNode *int_expr) override;
  void visitAdd(AddNode *add) override;
  void visitMul(MulNode *mul) override;
  void visitVar(VarNode *var) override;
  void visitAccess(AccessNode *access) override;
  void visitAssign(AssignmentNode *assign) override;
  void visitTensor(TensorNode *tensor) override;
  void visitFor(ForNode *loop) override;
};

class IRMutatorVisitor : public IRVisitor {
 public:
  IRNode *replaceFrom = nullptr;
  IRNode *replaceTo = nullptr;
  IRMutatorVisitor(IRNode *replaceFrom, IRNode *replaceTo)
      : replaceFrom(replaceFrom), replaceTo(replaceTo) {}

  template <typename T>
  T *_replace_subnode_helper(T *op_node, bool skipReplace = false);

  void visitInt(IntNode *int_expr) override;
  void visitAdd(AddNode *add) override;
  void visitMul(MulNode *mul) override;
  void visitVar(VarNode *var) override;
  void visitAccess(AccessNode *access) override;
  void visitAssign(AssignmentNode *assign) override;
  void visitTensor(TensorNode *tensor) override;
  void visitFor(ForNode *loop) override;
};

}  // namespace polly