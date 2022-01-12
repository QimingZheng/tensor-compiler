#pragma once

#include "common.h"

#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "polyhedral_model.h"

namespace polly {

class PolyhedralExtraction : public IRVisitor {
 public:
  PolyhedralExtraction() {}
  PolyhedralExtraction(IRHandle func) { func.accept(this); }

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

  std::vector<int> progContext;
  PolyhedralModel model;
  // affine access to arrays in a single statement
  std::vector<std::pair<ArrayKey, std::vector<QuasiAffineExpr>>> affineAccesses;
  QuasiAffineExpr expr;
  // affine iterations space.
  std::vector<Iteration> loops;
};

}  // namespace polly