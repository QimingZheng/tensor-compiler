#pragma once

#include "common.h"
#include "ir.h"
#include "pass/check/divisible_boundary_check.h"

namespace polly {

/// IRWorkSpace is a resource manager for the extracted IR Tree.
/// It contains the representation for the whole program.
class IRWorkSpace {
 public:
  IRWorkSpace() { root_ = NullIRHandle; }

  IRWorkSpace(IRHandle root, std::vector<IRHandle> tensors,
              std::vector<IRHandle> constants) {
    for (int i = 0; i < tensors.size(); i++) {
      tensors_.push_back(tensors[i].clone(irHandleDict_));
    }
    for (int i = 0; i < constants.size(); i++) {
      constants_.push_back(constants[i].clone(irHandleDict_));
    }
    root_ = root.clone(irHandleDict_);
  }
  ~IRWorkSpace() {}

  IRWorkSpace(const IRWorkSpace& other) {
    root_ = other.root_;
    tensors_ = other.tensors_;
    constants_ = other.constants_;
    reorderSchedules = other.reorderSchedules;
    fuseSchedules = other.fuseSchedules;
    splitSchedules = other.splitSchedules;
  }
  IRWorkSpace& operator=(const IRWorkSpace& other) {
    root_ = other.root_;
    tensors_ = other.tensors_;
    constants_ = other.constants_;
    reorderSchedules = other.reorderSchedules;
    fuseSchedules = other.fuseSchedules;
    splitSchedules = other.splitSchedules;
  }
  IRWorkSpace(IRWorkSpace&& other) {
    root_ = std::move(other.root_);
    tensors_ = std::move(other.tensors_);
    constants_ = std::move(other.constants_);
    reorderSchedules = other.reorderSchedules;
    fuseSchedules = other.fuseSchedules;
    splitSchedules = other.splitSchedules;
  }
  IRWorkSpace& operator=(IRWorkSpace&& other) {
    root_ = std::move(other.root_);
    tensors_ = std::move(other.tensors_);
    constants_ = std::move(other.constants_);
    reorderSchedules = other.reorderSchedules;
    fuseSchedules = other.fuseSchedules;
    splitSchedules = other.splitSchedules;
  }

  IRWorkSpace CreateSubSpace() {
    IRWorkSpace subspace(root_, tensors_, constants_);
    subspace.reorderSchedules = reorderSchedules;
    subspace.fuseSchedules = fuseSchedules;
    subspace.splitSchedules = splitSchedules;
    return subspace;
  }

  IRHandle& GetRoot() { return root_; }
  std::vector<IRHandle>& GetTensors() { return tensors_; }

  bool Reorder(const std::string i, const std::string j);
  bool Fuse(const std::string i, const std::string j, const std::string fuse);
  // Divide the i loop into `tiles` tiles.
  bool Split(const std::string i, IRHandle tiles, const std::string i_outter,
             const std::string i_inner);
  bool Unroll();

  /// Create Schedules With these Interfaces.
  bool CreateSplitSchedule(std::string axis);
  bool CreateReorderSchedule(std::string axis1, std::string axis2);
  bool CreateFuseSchedule(std::string axis1, std::string axix2);

  void RandomReorder() {
    if (reorderSchedules.size() == 0) return;
    int r = rand() % reorderSchedules.size();
    Reorder(reorderSchedules[r].first, reorderSchedules[r].second);
  }

  void RandomFuse() {
    if (fuseSchedules.size() == 0) return;
    int r = rand() % fuseSchedules.size();
    Fuse(fuseSchedules[r].first, fuseSchedules[r].second,
         fuseSchedules[r].first + "_" + fuseSchedules[r].second);
  }
  void RandomSplit() {
    if (splitSchedules.size() == 0) return;
    int r = rand() % splitSchedules.size();
    std::string splitAxis = splitSchedules[r];

    std::vector<int> divisors = {128, 64, 32, 16, 8, 4, 2};
    auto rng = std::default_random_engine{rand()};
    std::shuffle(divisors.begin(), divisors.end(), rng);
    for (auto i : divisors) {
      DivisibleBoundaryCheck checker(GetLoop(splitAxis), i);
      if (!checker.Check()) continue;
      IRHandle tiles = IntNode::make(i);
      Split(splitAxis, tiles, splitAxis + "_outter", splitAxis + "_inner");
      return;
    }
  }

  IRHandle GetLoop(std::string loopVarName) {
    return find_loop_var(root_, loopVarName);
  }

 private:
  /// Find the for-loop that uses var `loop_var_name` as its looping var.
  IRHandle find_loop_var(IRHandle cur, const std::string loop_var_name);

  int isNestedLoop(IRHandle outter, IRHandle inner);

  void evolveSplitSchedules(std::string axis, std::string axis1,
                            std::string axis2);
  void evolveReorderSchedules(std::string axis1, std::string axis2);
  void evolveFuseSchedules(std::string axis1, std::string axis2,
                           std::string axis);

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