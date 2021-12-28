#pragma once

#include "common.h"
#include "transform_pass.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

namespace polly {

class LoopVectorization : public TransformPass, public IRVisitor {
 public:
};

}  // namespace polly
