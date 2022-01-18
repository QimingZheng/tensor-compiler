#pragma once

#include "common.h"
#include "value_util.h"

namespace polly {

namespace isl {

class value : public object<isl_val> {
 public:
  value(isl_val *v) : object(v) {}
  value(const context &ctx, long v)
      : object(ctx, isl_val_int_from_si(ctx.get(), v)) {}
  value(const context &ctx, unsigned long v)
      : object(ctx, isl_val_int_from_ui(ctx.get(), v)) {}
  value(const context &ctx, int v)
      : object(ctx, isl_val_int_from_si(ctx.get(), v)) {}
  value(const context &ctx, unsigned int v)
      : object(ctx, isl_val_int_from_ui(ctx.get(), v)) {}
  long integer() const {
    assert(is_integer());
    return numerator();
  }
  bool is_integer() const { return isl_val_is_int(get()); }
  long numerator() const { return isl_val_get_num_si(get()); }
};
}  // namespace isl
}