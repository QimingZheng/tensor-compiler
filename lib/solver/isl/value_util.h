#pragma once

#include "common.h"

#include "object.h"
#include "context.h"

namespace polly {

namespace isl {

template <>
struct object_behavior<isl_val> {
  static isl_val *copy(isl_val *obj) { return isl_val_copy(obj); }
  static void destroy(isl_val *obj) { isl_val_free(obj); }
  static isl_ctx *get_context(isl_val *obj) { return isl_val_get_ctx(obj); }
  static void dump(isl_val *obj) { isl_val_dump(obj); }
};

}  // namespace isl
}