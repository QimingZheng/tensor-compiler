#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "ir/ir_module.h"
#include "transform_pass.h"

namespace polly {

class FissionTransform : public TransformPass, public IRVisitor {
 public:
  FissionTransform(IRModule module, IRHandle targetLoop)
      : module_(module), loop_(targetLoop) {
    searching_ = true;
  }

  void Transform() override;

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

  IRModule module_;
  IRHandle loop_;
  IRHandle replace_loop_;
  bool searching_;
};

}  // namespace polly