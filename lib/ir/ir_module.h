/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:32:39
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-19 20:28:45
 * @CopyRight: Qiming Zheng
 */
#pragma once

#include "common.h"
#include "ir.h"
#include "pass/check/divisible_boundary_check.h"

namespace polly {

/*!
 * \brief IRModule is a resource manager for the extracted IR Tree. It contains
 * the representation for the whole program. IRModule provides basic utilities
 * to operate upon the IR Tree.
 *
 * \param root The root node of the represented program.
 * \param tensors All tensors used in the program.
 * \param constants All constants used in the program.
 */
class IRModule {
 public:
  IRModule() { root_ = NullIRHandle; }

  IRModule(IRHandle root, std::vector<IRHandle> tensors,
           std::vector<IRHandle> constants) {
    for (int i = 0; i < tensors.size(); i++) {
      tensors_.push_back(tensors[i].clone(irHandleDict_));
    }
    for (int i = 0; i < constants.size(); i++) {
      constants_.push_back(constants[i].clone(irHandleDict_));
    }
    root_ = root.clone(irHandleDict_);
  }
  ~IRModule() {}

  IRModule(const IRModule& other) {
    root_ = other.root_;
    tensors_ = other.tensors_;
    constants_ = other.constants_;
    reorderSchedules = other.reorderSchedules;
    fuseSchedules = other.fuseSchedules;
    splitSchedules = other.splitSchedules;
  }
  IRModule& operator=(const IRModule& other) {
    root_ = other.root_;
    tensors_ = other.tensors_;
    constants_ = other.constants_;
    reorderSchedules = other.reorderSchedules;
    fuseSchedules = other.fuseSchedules;
    splitSchedules = other.splitSchedules;
  }
  IRModule(IRModule&& other) {
    root_ = std::move(other.root_);
    tensors_ = std::move(other.tensors_);
    constants_ = std::move(other.constants_);
    reorderSchedules = other.reorderSchedules;
    fuseSchedules = other.fuseSchedules;
    splitSchedules = other.splitSchedules;
  }
  IRModule& operator=(IRModule&& other) {
    root_ = std::move(other.root_);
    tensors_ = std::move(other.tensors_);
    constants_ = std::move(other.constants_);
    reorderSchedules = other.reorderSchedules;
    fuseSchedules = other.fuseSchedules;
    splitSchedules = other.splitSchedules;
  }

  IRModule CreateSubSpace() {
    IRModule subspace(root_, tensors_, constants_);
    subspace.reorderSchedules = reorderSchedules;
    subspace.fuseSchedules = fuseSchedules;
    subspace.splitSchedules = splitSchedules;
    return subspace;
  }

  IRHandle& GetRoot() { return root_; }
  std::vector<IRHandle>& GetTensors() { return tensors_; }

  /// Extract all IRNodes from a program.
  std::unordered_set<IRHandle, IRHandleHash> GetIRNodes();

  /// Create Schedules With these Interfaces.
  bool CreateSplitSchedule(std::string axis);
  bool CreateReorderSchedule(std::string axis1, std::string axis2);
  bool CreateFuseSchedule(std::string axis1, std::string axix2);

  IRHandle GetLoop(std::string loopVarId) { return find_loop_var(loopVarId); }

 private:
  IRHandle find_loop_var(const std::string loop_var_id);

  /// Find the for-loop that uses var `loop_var_id` as its looping var.
  IRHandle _find_loop_var(IRHandle cur, const std::string loop_var_id);

  int isNestedLoop(IRHandle outter, IRHandle inner);

  std::map<std::string, IRHandle> irHandleDict_;

  IRHandle root_;
  std::vector<IRHandle> tensors_;
  std::vector<IRHandle> constants_;

  std::vector<std::string> unrollSchedules;
  std::vector<std::pair<std::string, std::string>> reorderSchedules;
  std::vector<std::pair<std::string, std::string>> fuseSchedules;
  std::vector<std::string> splitSchedules;
};

}  // namespace polly