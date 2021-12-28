#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
// #include "ir/ir_check_pass.h"
#include "expr.h"
#include "stmt.h"
#include "codegen/codegen.h"
#include "ir/ir_workspace.h"
#include "jit/jit_module.h"

#include "pass/pass.h"
#include "pass/optimization/constant_folding.h"
#include "pass/check/affine_check.h"
#include "pass/check/constant_boundary_check.h"
#include "pass/check/divisible_boundary_check.h"

#include "auto_scheduler/auto_scheduler.h"

namespace polly {

/// Should be Singleton.
class Program {
 private:
  std::stack<IRHandle> loopScopes_;

  IRWorkSpace workspace_;

  IRHandle current_loop_;

  AutoScheduler scheduler_;

  /// Find the for-loop that uses var `loop_var_name` as its looping var.
  IRHandle find_loop_var(IRHandle cur, const std::string loop_var_name) {
    ForHandle curFor = cur.as<ForNode>();
    if (curFor == nullptr) return NullIRHandle;
    if (curFor->looping_var_.as<VarNode>()->name == loop_var_name) {
      return cur;
    }
    for (int i = 0; i < curFor->body.size(); i++) {
      if (curFor->body[i].Type() == IRNodeType::FOR) {
        IRHandle ret = find_loop_var(curFor->body[i], loop_var_name);
        if (ret != NullIRHandle) return ret;
      }
    }
    return NullIRHandle;
  }

  int isNestedLoop(IRHandle outter, IRHandle inner) {
    ForHandle outterFor = outter.as<ForNode>();
    ForHandle innerFor = inner.as<ForNode>();
    if (outterFor == nullptr) return false;
    if (innerFor == nullptr) return false;
    for (int i = 0; i < outterFor->body.size(); i++) {
      if (outterFor->body[i].Type() == IRNodeType::FOR) {
        if (outterFor->body[i].equals(inner)) return i;
      }
    }
    return -1;
  }

 protected:
  static Program *singleton_;
  std::string value_;

 public:
  Program() {
    if (singleton_ != nullptr)
      throw std::runtime_error("A program is already created\n");
    singleton_ = this;
  }

  ~Program() { singleton_ = nullptr; }
  Program(Program &other) = delete;
  void operator=(const Program &) = delete;

  static Program *GetInstance();

  bool EnterLoop(const Variable *var) {
    if (workspace_.GetRoot() == NullIRHandle) {
      workspace_.GetRoot() = ForNode::make(var->GetIRHandle());
      loopScopes_.push(workspace_.GetRoot());
    } else {
      IRHandle loop = ForNode::make(var->GetIRHandle());
      loopScopes_.push(loop);
      current_loop_.as<ForNode>()->Insert(loop);
    }
    current_loop_ = loopScopes_.top();
    return false;
  }

  bool AddStmt(Stmt *stmt) {
    IRPrinterVisitor visitor;
    current_loop_.as<ForNode>()->Insert(stmt->GetIRHandle());
    return false;
  }

  bool ExitLoop(const Variable *var) {
    loopScopes_.pop();
    if (!loopScopes_.empty()) current_loop_ = loopScopes_.top();
    return false;
  }

  /// The following interfaces create schedules
  bool SetReorder(const std::string i, const std::string j);

  bool SetFuse(const std::string i, const std::string j);

  // Divide the i loop into `tiles` tiles.
  bool SetSplit(const std::string i);

  bool SetParallel(const std::string i) { return false; }

  bool SetVectorize(const std::string i, int vectorLength) { return false; }

  bool SetUnroll();

  /// The following interfaces directly execute schedules.
  bool Reorder(const std::string i, const std::string j);

  bool Fuse(const std::string i, const std::string j);

  // Divide the i loop into `tiles` tiles.
  bool Split(const std::string i, Expr tiles);

  bool Unroll();

  bool Vectorize(const std::string i, int vectorLength) { return false; }

  void AutoTune() {
    scheduler_.Search(workspace_);
    for (int i = 0; i < 10; i++) {
      scheduler_.Search();
    }
    workspace_ = scheduler_.best_workspace_;
  }

  void IRGen() {
    IRPrinterVisitor visitor;
    visitor.visit(workspace_.GetRoot());
  }

  bool IsAffineProgram() {
    AffineCheck check(workspace_.GetRoot());
    return check.Check();
  }

  bool IsConstantBoundary() {
    ConstantBoundaryCheck checker(workspace_.GetRoot());
    return checker.Check();
  }

  bool IsBoundaryDivisible(std::string i, int divisor) {
    auto loop = workspace_.GetLoop(i);
    assert(loop != NullIRHandle);
    DivisibleBoundaryCheck checker(loop, divisor);
    return checker.Check();
  }

  void GenerateC() {
    ConstantFoldingPass confold(workspace_.GetRoot());
    confold.Optimize();
    CodeGenC codegen(std::cout);
    codegen.genCode(workspace_.GetRoot(), workspace_.GetTensors());
  }

  void DeclareTensor(Tensor *tensor) {
    workspace_.GetTensors().push_back(
        static_cast<IRHandle>(tensor->GetIRHandle()));
  }

  void RunJit() {
    JitModule jit(workspace_);
    jit.execute();
  }
};

}  // namespace polly