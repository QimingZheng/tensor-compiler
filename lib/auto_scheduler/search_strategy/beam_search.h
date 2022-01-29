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

namespace polly {

/// A beam search base searching strategy.
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

  IRModule Search(IRModule module = IRModule()) override;
};

}  // namespace polly
