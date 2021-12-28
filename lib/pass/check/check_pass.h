#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "pass/pass.h"

namespace polly {

class CheckPass : public Pass {
 public:
  CheckPass() {}
  virtual bool Check() = 0;
};

}  // namespace polly