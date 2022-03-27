/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:32:24
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-26 19:15:07
 * @CopyRight: Qiming Zheng
 */
#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "expr.h"
#include "stmt.h"
#include "codegen/codegen.h"
#include "ir/ir_module.h"
#include "jit/jit_module.h"

#include "pass/pass.h"
#include "pass/optimization/constant_folding.h"
#include "pass/check/affine_check.h"
#include "pass/check/constant_boundary_check.h"
#include "pass/check/divisible_boundary_check.h"

#include "auto_scheduler/auto_scheduler.h"

namespace polly {

/*!
 * \brief A program manage all the computation logics that users described
 * through their DSL code. `Program` is implemented as a singleton for now.
 */
class Program {
 public:
  IRModule module_;

 private:
  /// The outter nesting loops that encloses current bodies.
  std::stack<IRHandle> loopScopes_;

  /// A temporary variable used to represent the current loop.
  IRHandle current_loop_;

  /// The auto scheduler used to tune program.
  AutoScheduler scheduler_;

 protected:
  /// The singleton instance.
  static Program *singleton_;
  /// ???
  std::string value_;
  /// The name of the program, e.g. matmatmul, lu_decomposition, 2dconvolution
  std::string program_name_;

 public:
  Program(std::string program_name = "undefined")
      : program_name_(program_name) {
    if (singleton_ != nullptr)
      throw std::runtime_error("A program is already created\n");
    singleton_ = this;
  }

  ~Program() { singleton_ = nullptr; }
  Program(Program &other) = delete;
  void operator=(const Program &) = delete;

  static Program *GetInstance();

  // Every time an Expr::Variable is created, it will call the enter loop to
  // create a new level of looping.
  bool EnterLoop(const Variable *var) {
    if (module_.GetRoot() == NullIRHandle) {
      module_.GetRoot() = FuncNode::make({});
    }
    if (loopScopes_.empty()) {
      IRHandle loop = ForNode::make(var->GetIRHandle());
      module_.GetRoot().as<FuncNode>()->body.push_back(loop);
      loopScopes_.push(loop);
    } else {
      IRHandle loop = ForNode::make(var->GetIRHandle());
      loopScopes_.push(loop);
      current_loop_.as<ForNode>()->Insert(loop);
    }
    current_loop_ = loopScopes_.top();

    return false;
  }
  // Get the outter looping vars of current body.
  std::vector<IRHandle> GetLoopingVars() {
    std::vector<IRHandle> ret;
    for (int i = 0; i < loopScopes_.size(); i++) {
      ret.push_back(loopScopes_.top());
      loopScopes_.pop();
    }
    std::reverse(ret.begin(), ret.end());
    for (int i = 0; i++; i++) {
      loopScopes_.push(ret[i]);
    }
    return ret;
  }

  // Every time a Stmt is created, it will call the AddStmt to
  // insert a statment to current scope.
  bool AddStmt(Stmt *stmt) {
    if (module_.GetRoot() == NullIRHandle) {
      module_.GetRoot() = FuncNode::make({});
    }
    if (loopScopes_.empty()) {
      module_.GetRoot().as<FuncNode>()->body.push_back(stmt->GetIRHandle());
    } else {
      current_loop_.as<ForNode>()->Insert(stmt->GetIRHandle());
    }
    return false;
  }

  /// When an Expr::Variable is deconstructed, it means that the loop level has
  /// finished, just return to its parent looping scope.
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

  bool Vectorize(const std::string i, int vectorLength) { return false; }

  void AutoTune(std::string searching_strategy = "BeamSearch",
                int random_search_steps = 100,
                ArchSpec arch = ArchSpec(ArchSpec::ArchType::CPU)) {
    if (searching_strategy == "BeamSearch")
      module_ = scheduler_.BeamSearch(module_, arch, program_name_);
    else if (searching_strategy == "RandomSearch")
      module_ = scheduler_.RandomSearch(module_, random_search_steps, arch,
                                        program_name_);
    else if (searching_strategy == "HeuristicSearch")
      module_ = scheduler_.HeuristicSearch(module_, arch, program_name_);
  }

  void IRGen() {
    IRPrinterVisitor visitor;
    visitor.visit(module_.GetRoot());
  }

  bool IsAffineProgram() {
    auto ret = AffineCheck::runPass(std::shared_ptr<AffineCheck::Arg>(
        new AffineCheck::Arg(module_.GetRoot())));
    return PassRet::as<AffineCheck::Ret>(ret)->isAffine;
  }

  bool IsConstantBoundary() {
    auto ret = ConstantBoundaryCheck::runPass(
        std::shared_ptr<ConstantBoundaryCheck::Arg>(
            new ConstantBoundaryCheck::Arg(module_.GetRoot())));
    return PassRet::as<ConstantBoundaryCheck::Ret>(ret)->isConstantBoundary;
  }

  bool IsBoundaryDivisible(std::string i, int divisor) {
    auto loop = module_.GetLoop(i);
    assert(loop != NullIRHandle);
    auto ret = DivisibleBoundaryCheck::runPass(
        std::shared_ptr<DivisibleBoundaryCheck::Arg>(
            new DivisibleBoundaryCheck::Arg(loop, divisor)));
    return PassRet::as<DivisibleBoundaryCheck::Ret>(ret)->isDivisibleBoundary;
  }

  void GenerateC() {
    // ConstantFoldingPass::runPass(std::shared_ptr<ConstantFoldingPass::Arg>(
    //     new ConstantFoldingPass::Arg(module_.GetRoot())));
    CodeGenC codegen;
    std::cout << codegen.genCode(module_.GetRoot(), module_.GetTensors(),
                                 program_name_);
  }

  void GenerateCuda() {
    CodeGenCuda codegen;
    std::cout << codegen.genCode(module_.GetRoot(), module_.GetTensors());
  }

  void DeclareTensor(Tensor *tensor) {
    module_.GetTensors().push_back(
        static_cast<IRHandle>(tensor->GetIRHandle()));
  }

  void RunJit() {
    JitModule jit(module_);
    jit.execute();
  }
};

}  // namespace polly