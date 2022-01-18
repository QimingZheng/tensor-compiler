#pragma once

#include "common.h"
#include "context.h"
#include "object.h"

namespace polly {

namespace isl {

template <>
struct object_behavior<isl_space> {
  static isl_space *copy(isl_space *obj) { return isl_space_copy(obj); }
  static void destroy(isl_space *obj) { isl_space_free(obj); }
  static isl_ctx *get_context(isl_space *obj) { return isl_space_get_ctx(obj); }
  static void dump(isl_space *obj) { isl_space_dump(obj); }
};

template <>
struct object_behavior<isl_local_space> {
  static isl_local_space *copy(isl_local_space *obj) {
    return isl_local_space_copy(obj);
  }
  static void destroy(isl_local_space *obj) { isl_local_space_free(obj); }
  static isl_ctx *get_context(isl_local_space *obj) {
    return isl_local_space_get_ctx(obj);
  }
  static void dump(isl_local_space *obj) { isl_local_space_dump(obj); }
};
}  // namespace isl
}