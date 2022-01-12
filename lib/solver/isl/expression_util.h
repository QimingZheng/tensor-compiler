#pragma once

#include "common.h"
#include "object.h"
#include "context.h"

namespace polly {

namespace isl {

template <>
struct object_behavior<isl_aff> {
  static isl_aff *copy(isl_aff *obj) { return isl_aff_copy(obj); }
  static void destroy(isl_aff *obj) { isl_aff_free(obj); }
  static isl_ctx *get_context(isl_aff *obj) { return isl_aff_get_ctx(obj); }
  static void dump(isl_aff *obj) { isl_aff_dump(obj); }
};

template <>
struct object_behavior<isl_pw_aff> {
  static isl_pw_aff *copy(isl_pw_aff *obj) { return isl_pw_aff_copy(obj); }
  static void destroy(isl_pw_aff *obj) { isl_pw_aff_free(obj); }
  static isl_ctx *get_context(isl_pw_aff *obj) {
    return isl_pw_aff_get_ctx(obj);
  }
  static void dump(isl_pw_aff *obj) { isl_pw_aff_dump(obj); }
};

template <>
struct object_behavior<isl_multi_aff> {
  static isl_multi_aff *copy(isl_multi_aff *obj) {
    return isl_multi_aff_copy(obj);
  }
  static void destroy(isl_multi_aff *obj) { isl_multi_aff_free(obj); }
  static isl_ctx *get_context(isl_multi_aff *obj) {
    return isl_multi_aff_get_ctx(obj);
  }
  static void dump(isl_multi_aff *obj) { isl_multi_aff_dump(obj); }
};
}  // namespace isl
}