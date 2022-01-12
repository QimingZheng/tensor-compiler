#pragma once

#include "common.h"
#include "space.h"
#include "constraint_util.h"
#include "value.h"
#include "expression.h"

namespace polly {

namespace isl {

class constraint : public object<isl_constraint> {
 public:
  constraint(isl_constraint *ptr) : object(ptr) {}

  static constraint equality(const local_space &spc) {
    return isl_equality_alloc(spc.copy());
  }
  static constraint inequality(const local_space &spc) {
    return isl_inequality_alloc(spc.copy());
  }
  static constraint equality(const expression &expr) {
    return isl_equality_from_aff(expr.copy());
  }
  static constraint inequality(const expression &expr) {
    return isl_inequality_from_aff(expr.copy());
  }

  bool is_equality() const { return isl_constraint_is_equality(get()); }

  value get_constant() { return isl_constraint_get_constant_val(m_object); }

  value get_coefficient(isl_dim_type dim, int i) {
    return isl_constraint_get_coefficient_val(m_object, dim, i);
  }

  void set_coefficient(space::dimension_type type, int pos, int val) {
    m_object = isl_constraint_set_coefficient_si(m_object, (isl_dim_type)type,
                                                 pos, val);
  }
  void set_coefficient(space::dimension_type type, int pos, const value &val) {
    m_object = isl_constraint_set_coefficient_val(m_object, (isl_dim_type)type,
                                                  pos, val.copy());
  }
  void set_constant(int val) {
    m_object = isl_constraint_set_constant_si(m_object, val);
  }
  void set_constant(const value &val) {
    m_object = isl_constraint_set_constant_val(m_object, val.copy());
  }
  isl::expression expression() const { return isl_constraint_get_aff(get()); }
  isl::local_space local_space() const {
    return isl_constraint_get_local_space(get());
  }
  isl::space get_space() const { return isl_constraint_get_space(get()); }
};

class basic_set;

class constraint_list : public object<isl_constraint_list> {
 public:
  constraint_list(isl_constraint_list *ptr) : object(ptr) {}
  constraint_list(basic_set s);
  static constraint_list from_basic_set(basic_set s);
  int size() { return isl_constraint_list_size(m_object); }
  constraint at(int i) { return isl_constraint_list_get_at(m_object, i); }
  void drop(int i) { m_object = isl_constraint_list_drop(m_object, i, 1); }
};

inline constraint operator>=(const expression &lhs, const expression &rhs) {
  return constraint::inequality(lhs - rhs);
}

inline constraint operator>=(const expression &lhs, int rhs_int) {
  return constraint::inequality(lhs - rhs_int);
}

inline constraint operator>(const expression &lhs, const expression &rhs) {
  return constraint::inequality(lhs - rhs - 1);
}

inline constraint operator>(const expression &lhs, int rhs_int) {
  return constraint::inequality(lhs - (rhs_int + 1));
}

inline constraint operator<=(const expression &lhs, const expression &rhs) {
  return constraint::inequality(rhs - lhs);
}

inline constraint operator<=(const expression &lhs, int rhs_int) {
  return constraint::inequality(rhs_int - lhs);
}

inline constraint operator<(const expression &lhs, const expression &rhs) {
  return constraint::inequality(rhs - lhs - 1);
}

inline constraint operator<(const expression &lhs, int rhs_int) {
  return constraint::inequality((rhs_int - 1) - lhs);
}

inline constraint operator==(const expression &lhs, const expression &rhs) {
  return constraint::equality(lhs - rhs);
}

inline constraint operator==(const expression &lhs, const value &rhs) {
  return constraint::equality(lhs - rhs);
}

inline constraint operator==(const expression &lhs, int rhs_int) {
  return constraint::equality(lhs - rhs_int);
}

}  // namespace isl
}