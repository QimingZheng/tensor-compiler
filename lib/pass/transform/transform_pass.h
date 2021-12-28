#pragma once

#include "common.h"
#include "pass/pass.h"

namespace polly {

class TransformPass : public Pass {
 public:
  TransformPass() {}
  virtual void Transform() = 0;
};

}  // namespace polly