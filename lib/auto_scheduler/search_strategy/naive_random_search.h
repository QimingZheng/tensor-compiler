/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-26 13:10:55
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-26 19:11:05
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "auto_scheduler/cost_model.h"
#include "ir/ir_module.h"
#include "auto_scheduler/mutator/mutator.h"
#include "strategy.h"

namespace polly {

class RandomSearchStrategy : public SearchStrategy {
 public:
  RandomSearchStrategy(int search_budget = 10)
      : search_budget_(search_budget) {}

  int search_budget_;

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