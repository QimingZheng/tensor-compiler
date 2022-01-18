#pragma once

#include "common.h"
#include "object.h"

namespace polly {

namespace isl {

template <>
struct object_behavior<isl_basic_set> {
  static isl_basic_set *copy(isl_basic_set *obj) {
    return isl_basic_set_copy(obj);
  }
  static void destroy(isl_basic_set *obj) { isl_basic_set_free(obj); }
  static isl_ctx *get_context(isl_basic_set *obj) {
    return isl_basic_set_get_ctx(obj);
  }
  static void dump(isl_basic_set *obj) { isl_basic_set_dump(obj); }
};

template <>
struct object_behavior<isl_set> {
  static isl_set *copy(isl_set *obj) { return isl_set_copy(obj); }
  static void destroy(isl_set *obj) { isl_set_free(obj); }
  static isl_ctx *get_context(isl_set *obj) { return isl_set_get_ctx(obj); }
  static void dump(isl_set *obj) { isl_set_dump(obj); }
};

template <>
struct object_behavior<isl_union_set> {
  static isl_union_set *copy(isl_union_set *obj) {
    return isl_union_set_copy(obj);
  }
  static void destroy(isl_union_set *obj) { isl_union_set_free(obj); }
  static isl_ctx *get_context(isl_union_set *obj) {
    return isl_union_set_get_ctx(obj);
  }
  static void dump(isl_union_set *obj) { isl_union_set_dump(obj); }
};

}  // namespace isl
}