#include "map.h"

namespace polly {

namespace isl {
map lex_lt(space sp) { return isl_map_lex_lt(sp.copy()); }

void map::add_constraint(const constraint &c) {
  // auto isl_c = c.copy();
  // if (c.local_space().is_wrapping())
  //   isl_c = isl_constraint_unwrap_local_space(isl_c);
  m_object = isl_map_add_constraint(m_object, c.copy());
}

}  // namespace isl
}