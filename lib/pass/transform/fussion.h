#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

namespace polly {

class FussionTransform : public IRVisitor {
 public:
  FussionTransform() {}
};

}  // namespace polly