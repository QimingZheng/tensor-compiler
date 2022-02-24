/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:29:40
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-26 19:22:33
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "auto_scheduler/cost_model/cost_model.h"
#include "ir/ir_module.h"
#include "search_strategy/beam_search.h"
#include "search_strategy/naive_random_search.h"

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

  IRModule BeamSearch(IRModule module, ArchSpec spec,
                      std::string program_name) {
    BeamSearchStrategy bs(4, 4, 10);
    return bs.Search(module, spec, program_name);
  }

  IRModule RandomSearch(IRModule module, int random_search_steps, ArchSpec spec,
                        std::string program_name) {
    RandomSearchStrategy rs(random_search_steps);
    return rs.Search(module, spec, program_name);
  }

 private:
};

}  // namespace polly
