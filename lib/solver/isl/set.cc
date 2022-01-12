#include "set.h"
#include "map.h"
#include "constraint.h"

namespace polly {

namespace isl {

basic_map basic_set::operator*(basic_set s) {
  return basic_map(isl_basic_map_from_domain_and_range(copy(), s.copy()));
}

map set::operator*(set s) {
  return map(isl_map_from_domain_and_range(copy(), s.copy()));
}

union_map union_set::operator*(union_set s) {
  return union_map(isl_union_map_from_domain_and_range(copy(), s.copy()));
}

union_map union_set::identity() {
  return union_map(isl_union_set_identity(copy()));
}

map set::identity() { return map(isl_set_identity(copy())); }

basic_map basic_set::unwrap() { return isl_basic_set_unwrap(copy()); }
map set::unwrap() { return isl_set_unwrap(copy()); }
union_map union_set::unwrap() { return isl_union_set_unwrap(copy()); }

void set::add_constraint(const constraint &c) {
  m_object = isl_set_add_constraint(m_object, c.copy());
}

void basic_set::add_constraint(const constraint &c) {
  m_object = isl_basic_set_add_constraint(m_object, c.copy());
}
}  // namespace isl
}