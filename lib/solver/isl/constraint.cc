#include "common.h"
#include "constraint.h"
#include "space.h"
#include "expression.h"
#include "set.h"

namespace polly {

namespace isl {

constraint_list::constraint_list(basic_set s)
    : object(s.ctx(), isl_basic_set_get_constraint_list(s.get())) {}

constraint_list constraint_list::from_basic_set(basic_set set) {
  return constraint_list(isl_basic_set_get_constraint_list(set.get()));
}
}  // namespace isl
}