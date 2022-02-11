/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:29:51
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-28 16:15:40
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"

#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "polyhedral_model.h"

namespace polly {

/*!
 * \brief PolyhedralExtraction extracts the polyhedral model representation of
 * each statement in the program. Constant Folding/Propagation pass must run
 * before ezxtraction to handle the min/max expressions.
 *
 * \param func The root node of the represented program (i.e. a FuncNode).
 */
class PolyhedralExtraction : public IRNotImplementedVisitor {
 public:
  PolyhedralExtraction() {}
  PolyhedralExtraction(IRHandle func) { func.accept(this); }
  // Extract the polyhedral model nested inside a certain node.
  PolyhedralExtraction(std::vector<IRHandle> enclosing_looping_vars,
                       std::vector<int> prog_ctx, IRHandle node) {
    progContext = prog_ctx;
    for (int i = 0; i < enclosing_looping_vars.size(); i++) {
      auto var = enclosing_looping_vars[i].as<VarNode>();

      workspace.clear();
      var->min.accept(this);
      auto min_ws = workspace;
      auto mins_ = workspace.max_exprs.size() > 0
                       ? workspace.max_exprs
                       : std::vector<QuasiAffineExpr>{workspace.expr};
      workspace.clear();
      var->max.accept(this);
      auto max_ws = workspace;
      auto maxs_ = workspace.min_exprs.size() > 0
                       ? workspace.min_exprs
                       : std::vector<QuasiAffineExpr>{workspace.expr};

      for (auto &max_ : maxs_) {
        // max_ is a non-inclusive bound
        // max_.constant -= 1;
        max_.constant -= 1 * max_.divisor;
      }

      loops.push_back(Iteration(var->id, mins_, maxs_));
    }
    node.accept(this);
  }

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
  void visitPrint(PrintHandle print) override;
  void visitFunc(FuncHandle func) override;

  void visitMin(MinHandle min) override;
  void visitMax(MaxHandle max) override;

  static QuasiAffineExpr IRHandleToQuasiAffine(IRHandle handle) {
    PolyhedralExtraction model(handle);
    return model.workspace.expr;
  }

  static IterDomain ExtractIterDomain(
      std::vector<IRHandle> enclosing_looping_vars) {
    IterDomain ret;
    for (int i = 0; i < enclosing_looping_vars.size(); i++) {
      auto var = enclosing_looping_vars[i].as<VarNode>();

      PolyhedralExtraction extractor;

      extractor.workspace.clear();
      // var->min.accept(this);
      extractor.visit(var->min);
      auto min_ws = extractor.workspace;
      auto mins_ = extractor.workspace.max_exprs.size() > 0
                       ? extractor.workspace.max_exprs
                       : std::vector<QuasiAffineExpr>{extractor.workspace.expr};
      extractor.workspace.clear();
      // var->max.accept(this);
      extractor.visit(var->max);
      auto max_ws = extractor.workspace;
      auto maxs_ = extractor.workspace.min_exprs.size() > 0
                       ? extractor.workspace.min_exprs
                       : std::vector<QuasiAffineExpr>{extractor.workspace.expr};

      for (auto &max_ : maxs_) {
        // max_ is a non-inclusive bound
        // max_.constant -= 1;
        max_.constant -= 1 * max_.divisor;
      }

      ret.iterations_.push_back(Iteration(var->id, mins_, maxs_));
    }
    return ret;
  }

  PolyhedralModel model;

  /// The following variables are used as workspace vars.

  std::vector<int> progContext;
  // affine access to arrays in a single statement
  std::vector<std::pair<ArrayKey, std::vector<QuasiAffineExpr>>> affineAccesses;
  // temporary workspace.
  struct WorkSpace {
    std::vector<QuasiAffineExpr> min_exprs;
    std::vector<QuasiAffineExpr> max_exprs;
    QuasiAffineExpr expr;
    void clear() {
      min_exprs.clear();
      max_exprs.clear();
      expr.clear();
    }
  };

  WorkSpace workspace;

  // affine iterations space.
  std::vector<Iteration> loops;
};

}  // namespace polly