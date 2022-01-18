#pragma once

#include "common.h"
#include "object.h"
#include "context.h"

namespace polly {

namespace isl {

template <>
struct object_behavior<isl_basic_map> {
  static isl_basic_map *copy(isl_basic_map *obj) {
    return isl_basic_map_copy(obj);
  }
  static void destroy(isl_basic_map *obj) { isl_basic_map_free(obj); }
  static isl_ctx *get_context(isl_basic_map *obj) {
    return isl_basic_map_get_ctx(obj);
  }

  static void dump(isl_basic_map *obj) { isl_basic_map_dump(obj); }
};

template <>
struct object_behavior<isl_map> {
  static isl_map *copy(isl_map *obj) { return isl_map_copy(obj); }
  static void destroy(isl_map *obj) { isl_map_free(obj); }
  static isl_ctx *get_context(isl_map *obj) { return isl_map_get_ctx(obj); }
  static void dump(isl_map *obj) { isl_map_dump(obj); }
};

template <>
struct object_behavior<isl_union_map> {
  static isl_union_map *copy(isl_union_map *obj) {
    return isl_union_map_copy(obj);
  }
  static void destroy(isl_union_map *obj) { isl_union_map_free(obj); }
  static isl_ctx *get_context(isl_union_map *obj) {
    return isl_union_map_get_ctx(obj);
  }
  static void dump(isl_union_map *obj) { isl_union_map_dump(obj); }
};
}  // namespace isl
}