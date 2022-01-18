#pragma once

#include "common.h"
#include "context.h"
#include "object.h"

namespace polly {

namespace isl {
template <>
struct object_behavior<isl_constraint> {
  static isl_constraint *copy(isl_constraint *obj) {
    return isl_constraint_copy(obj);
  }
  static void destroy(isl_constraint *obj) { isl_constraint_free(obj); }
  static isl_ctx *get_context(isl_constraint *obj) {
    return isl_constraint_get_ctx(obj);
  }
  static void dump(isl_constraint *obj) { isl_constraint_dump(obj); }
};

template <>
struct object_behavior<isl_constraint_list> {
  static isl_constraint_list *copy(isl_constraint_list *obj) {
    return isl_constraint_list_copy(obj);
  }
  static void destroy(isl_constraint_list *obj) {
    isl_constraint_list_free(obj);
  }
  static isl_ctx *get_context(isl_constraint_list *obj) {
    return isl_constraint_list_get_ctx(obj);
  }
  static void dump(isl_constraint_list *obj) { isl_constraint_list_dump(obj); }
};

}  // namespace isl
}  // namespace polly