#pragma once

#include "common.h"
#include "pass/pass.h"

namespace polly {

class OptimizationPass : public Pass {
 public:
  OptimizationPass() {}
  virtual void Optimize() = 0;
};

}  // namespace polly