#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_module.h"
#include "strategy.h"
#include "auto_scheduler/cost_model/cost_model.h"

namespace polly {

class MonteCarloSearchStrategy : public SearchStrategy {
  int search_trials;
  int expanding_size;
  size_t candidate_size;

 public:
  MonteCarloSearchStrategy(int search_trials, int expanding_size,
                           int candidate_size)
      : search_trials(search_trials),
        expanding_size(expanding_size),
        candidate_size(candidate_size) {}

  /*!
   * \brief A Monte Carlo Tree based search strategy. In each itearation, MCTS
   * expand searching branches from a set of candidate programs, then select
   * promising branches according to the DNN-based cost model estimations and
   * evaluate these branches by measuring with real hardware.
   *
   * \param module
   */
  IRModule Search(IRModule module, ArchSpec spec,
                  std::string program_name) override;

  IRModule Expand(IRModule module, ArchSpec spec, std::string program_name);
};

}  // namespace polly