#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

namespace polly {

/// Normalization Pass will transform an affine program into standarized form.
/// Including:
///     1. Make the incremental amount always equal to 1.
class NormalizationPass : public TransformPass, public IRVisitor {
 public:
  NormalizationPass() {}
};

}  // namespace polly
