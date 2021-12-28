#pragma once

#include "common.h"
#include "transform_pass.h"

namespace polly {

class LoopParallelization : public TransformPass, public IRVisitor {
 public:
};

}  // namespace polly