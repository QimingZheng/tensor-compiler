/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-26 10:11:04
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-26 19:22:21
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "auto_scheduler/cost_model/cost_model.h"
#include "ir/ir_module.h"
#include "strategy.h"
#include "auto_scheduler/mutator/mutator.h"

namespace polly {

/*!
 * \brief A beam search base program tunning and searching strategy.
 *
 * \param beam_search_width The number of sub-schedules expanded from each
 * candidates.
 * \param candidate_size The number of candidates survived after each searching
 * step.
 * \param search_budget Searching steps.
 */
class BeamSearchStrategy : public SearchStrategy {
 public:
  BeamSearchStrategy(int beam_search_width = 4, int candidate_size = 4,
                     int search_budget = 1)
      : beam_search_width_(beam_search_width),
        candidate_size_(candidate_size),
        search_budget_(search_budget) {}

  std::vector<IRModule> candidates;

  IRModule best_module_;
  float best_performance_ = 1000000;
  int search_budget_;
  /// how many sub-space to be search from each candidate
  int beam_search_width_;
  /// maximum of candidates at the same time
  int candidate_size_;

  /// randomly expand an IR module through any valid IR transform.
  std::vector<IRModule> Expand(IRModule module);

  void RandomSearch(IRModule &module);
  IRModule Search(IRModule module = IRModule()) override;

 private:
  IRHandle GetRandomLoop(std::unordered_set<IRHandle, IRHandleHash> &node_set) {
    auto nodes = std::vector<IRHandle>(node_set.begin(), node_set.end());

    auto rng = std::default_random_engine{rand()};
    std::shuffle(nodes.begin(), nodes.end(), rng);
    for (int i = 0; i < nodes.size(); i++) {
      if (nodes[i].Type() == IRNodeType::FOR) return nodes[i];
    }
    return NullIRHandle;
  }
};

}  // namespace polly
