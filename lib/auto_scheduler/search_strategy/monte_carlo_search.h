#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_module.h"
#include "strategy.h"

namespace polly {

class MonteCarloSearchStrategy : public SearchStrategy {
 public:
  MonteCarloSearchStrategy() {}

  IRModule Search(IRModule module = IRModule()) override;
};

}  // namespace polly