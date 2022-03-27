#pragma once

#include "common.h"
#include "auto_scheduler/cost_model/cost_model.h"
#include "ir/ir_module.h"
#include "strategy.h"
#include "auto_scheduler/mutator/mutator.h"
#include "search_history_log.h"

namespace polly {

/*
 * Heuristic Transformation Plan Search Rules:
 */

class HeuristicSearchStrategy : public SearchStrategy {
 public:
  HeuristicSearchStrategy(int candidate_size = 4, int search_budget = 1)
      : candidate_size_(candidate_size),
        search_budget_(search_budget),
        tree(nullptr) {}

  std::vector<SearchNodeHandle> candidates;

  IRModule best_module_;
  float best_performance_ = 1000000;
  int search_budget_;

  /// maximum of candidates at the same time
  int candidate_size_;

  /// randomly expand an IR module through any valid IR transform.
  void Expand(SearchNodeHandle &parent, SearchNodeHandle &child,
              std::string action);

  void RandomSearch(SearchNodeHandle parent, SearchNodeHandle child);
  IRModule Search(IRModule module, ArchSpec spec,
                  std::string program_name) override;

  // Estimation of spent cost.
  float cost(SearchNodeHandle &node);
  // Estimation of the quality of this searching path.
  float heuristic(SearchNodeHandle &node);

  float softmax(std::vector<SearchNodeHandle &> nodes);

  SearchTrees *tree;

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
