/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:29:51
 * @Last Modified by:   Qiming Zheng
 * @Last Modified time: 2022-01-18 20:29:51
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"

#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "polyhedral_model.h"

namespace polly {

class PolyhedralExtraction : public IRNotImplementedVisitor {
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
  void visitPrint(PrintHandle print) override;
  void visitFunc(FuncHandle func) override;

  static QuasiAffineExpr IRHandleToQuasiAffine(IRHandle handle) {
    PolyhedralExtraction model(handle);
    return model.expr;
  }

  PolyhedralModel model;

  /// The following variables are used as workspace vars.

  std::vector<int> progContext;
  // affine access to arrays in a single statement
  std::vector<std::pair<ArrayKey, std::vector<QuasiAffineExpr>>> affineAccesses;
  QuasiAffineExpr expr;
  // affine iterations space.
  std::vector<Iteration> loops;
};

}  // namespace polly