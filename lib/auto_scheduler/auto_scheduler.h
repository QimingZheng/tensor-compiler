#pragma once

#include "common.h"
#include "cost_model.h"
#include "ir/ir_module.h"

namespace polly {

/// A beam search auto scheduler
class AutoScheduler {
 public:
  AutoScheduler(int beam_search_width = 4, int candidate_size = 4)
      : beam_search_width_(beam_search_width),
        candidate_size_(candidate_size) {}

  std::vector<IRModule> candidates;

  IRModule best_module_;
  float best_performance_ = 1000000;

  /// how many sub-space to be search from each candidate
  int beam_search_width_;
  /// maximum of candidates at the same time
  int candidate_size_;

  /// One search step.
  void Search(IRModule module = IRModule()) {
    if (module.GetRoot() != NullIRHandle) {
      best_module_ = module;
      candidates.clear();
      candidates.push_back(best_module_);
    }
    std::vector<std::pair<IRModule, float>> childrens;
    for (int i = 0; i < candidates.size(); i++) {
      for (int j = 0; j < beam_search_width_; j++) {
        childrens.push_back({candidates[i].CreateSubSpace(), 100.0});
      }
    }
    for (int i = 0; i < childrens.size(); i++) {
      int seed = rand() % 3;
      switch (seed) {
        case 0: {
          childrens[i].first.RandomSplit();
          break;
        }
        case 1: {
          childrens[i].first.RandomReorder();
          break;
        }
        case 2: {
          childrens[i].first.RandomFuse();
          break;
        }
        default:
          throw std::runtime_error("out of random bound");
      }
    }
    CostModel model;
    std::transform(
        childrens.begin(), childrens.end(), childrens.begin(),
        [&](const std::pair<IRModule, float> &x) -> std::pair<IRModule, float> {
          return {x.first, model.Evaluate(x.first)};
        });
    std::sort(childrens.begin(), childrens.end(),
              [&](const std::pair<IRModule, float> &x,
                  const std::pair<IRModule, float> &y) {
                return x.second < y.second;
              });
    if (childrens.begin()->second < best_performance_) {
      best_module_ = childrens.begin()->first.CreateSubSpace();
    }
    candidates.clear();
    std::transform(childrens.begin(), childrens.end(),
                   std::back_inserter(candidates),
                   [](const std::pair<IRModule, float> &x) { return x.first; });
    candidates.resize(candidate_size_);
    return;
  }

 private:
};

}  // namespace polly
