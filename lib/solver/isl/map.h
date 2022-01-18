#pragma once

#include "common.h"
#include "map_util.h"
#include "util.h"
#include "set.h"
#include "value.h"
#include "expression.h"
#include "constraint.h"
#include "space.h"
namespace polly {

namespace isl {

class basic_map : public object<isl_basic_map> {
 public:
  basic_map(isl_basic_map *map) : object(map) {}
  basic_map(const space &spc)
      : object(spc.ctx(), isl_basic_map_empty(spc.copy())) {}
  basic_map(context &ctx, const std::string &text)
      : object(isl_basic_map_read_from_str(ctx.get(), text.c_str())) {}
  basic_map(const expression &expr)
      : object(expr.ctx(), isl_basic_map_from_aff(expr.copy())) {}

  explicit basic_map() : object(nullptr, nullptr) {}

  basic_map operator()(const basic_map &other) {
    return isl_basic_map_apply_range(copy(), other.copy());
  }
  basic_set operator()(const basic_set &domain) {
    return basic_set(isl_basic_set_apply(domain.copy(), copy()));
  }
  basic_map operator*(basic_set &domain) {
    return basic_map(isl_basic_map_intersect_domain(copy(), domain.copy()));
  }

  bool empty() { return isl_basic_map_is_empty(m_object); }
  bool is_single_valued() { return isl_basic_map_is_single_valued(m_object); }
  basic_set domain() { return basic_set(isl_basic_map_domain(copy())); }
  basic_set range() { return basic_set(isl_basic_map_range(copy())); }
  basic_map domain_projection() {
    return basic_map(isl_basic_map_domain_map(copy()));
  }
  basic_map range_projection() {
    return basic_map(isl_basic_map_range_map(copy()));
  }
  basic_set wrap() { return basic_set(isl_basic_map_wrap(copy())); }
  basic_map cross(basic_map s) {
    return basic_map(isl_basic_map_product(copy(), s.copy()));
  }
  basic_map domain_product(basic_map other) {
    return isl_basic_map_domain_product(copy(), other.copy());
  }
  basic_map range_product(basic_map other) {
    return isl_basic_map_range_product(copy(), other.copy());
  }

  static basic_map universe(const space &s) {
    return isl_basic_map_universe(s.copy());
  }
  local_space get_local_space() { return isl_basic_map_get_local_space(get()); }

  void add_constraint(const constraint &c) {
    auto isl_c = c.copy();

    // if (c.local_space().is_wrapping())
    //   isl_c = isl_constraint_unwrap_local_space(isl_c);

    m_object = isl_basic_map_add_constraint(m_object, isl_c);
  }
  space get_space() const { return space(isl_basic_map_get_space(get())); }
};

class map : public object<isl_map> {
 public:
  map(isl_map *map) : object(map) {}
  map(context &ctx, const std::string &text)
      : object(isl_map_read_from_str(ctx.get(), text.c_str())) {}
  map(const expression &expr)
      : object(expr.ctx(), isl_map_from_aff(expr.copy())) {}

  map operator^(int power) {
    value v(m_ctx, power);
    map ret = map(isl_map_fixed_power_val(copy(), v.get()));
    return ret;
  }
  map operator()(const map &other) {
    return isl_map_apply_range(copy(), other.copy());
  }
  set operator()(const set &domain) {
    return set(isl_set_apply(domain.copy(), copy()));
  }
  bool empty() { return isl_map_is_empty(m_object); }
  bool is_single_valued() { return isl_map_is_single_valued(m_object); }
  bool is_injective() { return isl_map_is_injective(m_object); }
  set domain() { return set(isl_map_domain(copy())); }
  set range() { return set(isl_map_range(copy())); }
  map operator*(set &domain) {
    return map(isl_map_intersect_domain(copy(), domain.copy()));
  }
  map operator-(set &domain) {
    return map(isl_map_subtract_domain(copy(), domain.copy()));
  }
  map domain_projection() { return map(isl_map_domain_map(copy())); }
  map range_projection() { return map(isl_map_range_map(copy())); }
  set wrap() { return isl_map_wrap(copy()); }
  map cross(map s) { return map(isl_map_product(copy(), s.copy())); }
  map domain_product(map other) {
    return isl_map_domain_product(copy(), other.copy());
  }
  map range_product(map other) {
    return isl_map_range_product(copy(), other.copy());
  }
  map factor_domain() { return isl_map_factor_domain(copy()); }
  space get_space() const { return space(isl_map_get_space(get())); }

  void add_constraint(const constraint &c);
};

map lex_lt(space sp);

class union_map : public object<isl_union_map> {
 public:
  union_map() : object(nullptr, nullptr) {}
  union_map(isl_union_map *map) : object(map) {}
  union_map(context &ctx, const std::string &text)
      : object(isl_union_map_read_from_str(ctx.get(), text.c_str())) {}
  union_map(const map &m) : object(m.ctx(), isl_union_map_from_map(m.copy())) {}
  union_map(const basic_map &m)
      : object(m.ctx(), isl_union_map_from_basic_map(m.copy())) {}

  union_map operator^(int power) {
    value v(m_ctx, power);
    union_map ret = union_map(isl_union_map_fixed_power_val(copy(), v.copy()));
    return ret;
  }
  union_map operator()(const union_map &other) {
    return union_map(isl_union_map_apply_range(copy(), other.copy()));
  }
  union_set operator()(const union_set &domain) {
    return union_set(isl_union_set_apply(domain.copy(), copy()));
  }
  bool empty() { return isl_union_map_is_empty(m_object); }
  bool is_single_valued() { return isl_union_map_is_single_valued(m_object); }
  bool is_injective() { return isl_union_map_is_injective(m_object); }
  union_set domain() { return union_set(isl_union_map_domain(copy())); }
  union_set range() { return union_set(isl_union_map_range(copy())); }
  union_map operator*(union_set &domain) {
    return union_map(isl_union_map_intersect_domain(copy(), domain.copy()));
  }
  union_map operator-(union_set &domain) {
    return union_map(isl_union_map_subtract_domain(copy(), domain.copy()));
  }
  union_map domain_projection() {
    return union_map(isl_union_map_domain_map(copy()));
  }
  union_map range_projection() {
    return union_map(isl_union_map_range_map(copy()));
  }
  union_set wrap() { return isl_union_map_wrap(copy()); }
  union_map cross(union_map s) {
    return union_map(isl_union_map_product(copy(), s.copy()));
  }
  union_map domain_product(union_map other) {
    return isl_union_map_domain_product(copy(), other.copy());
  }
  union_map range_product(union_map other) {
    return isl_union_map_range_product(copy(), other.copy());
  }
  union_map factor_domain() { return isl_union_map_factor_domain(copy()); }

  space get_space() const { return space(isl_union_map_get_space(get())); }

  void add_constraint(const constraint &c);
};

inline map operator|(const map &a, const map &b) {
  return map(isl_map_union(a.copy(), b.copy()));
}
inline map operator|(const basic_map &a, const basic_map &b) {
  return map(isl_basic_map_union(a.copy(), b.copy()));
}
inline union_map operator|(const union_map &a, const union_map &b) {
  return union_map(isl_union_map_union(a.copy(), b.copy()));
}

inline map operator&(const map &a, const map &b) {
  return map(isl_map_intersect(a.copy(), b.copy()));
}
inline basic_map operator&(const basic_map &a, const basic_map &b) {
  return basic_map(isl_basic_map_intersect(a.copy(), b.copy()));
}
inline union_map operator&(const union_map &a, const union_map &b) {
  return union_map(isl_union_map_intersect(a.copy(), b.copy()));
}

inline map operator-(const map &a, const map &b) {
  return map(isl_map_subtract(a.copy(), b.copy()));
}
inline union_map operator-(const union_map &a, const union_map &b) {
  return union_map(isl_union_map_subtract(a.copy(), b.copy()));
}

inline bool operator==(const map &a, const map &b) {
  return IslBool2Bool(isl_map_is_equal(a.get(), b.get()), "operator ==");
}
inline bool operator==(const basic_map &a, const basic_map &b) {
  return IslBool2Bool(isl_basic_map_is_equal(a.get(), b.get()), "operator ==");
}
inline bool operator==(const union_map &a, const union_map &b) {
  return IslBool2Bool(isl_union_map_is_equal(a.get(), b.get()), "operator ==");
}

inline bool operator<(const map &a, const map &b) {
  return IslBool2Bool(isl_map_is_strict_subset(a.get(), b.get()), "operator <");
}
inline bool operator<(const basic_map &a, const basic_map &b) {
  return IslBool2Bool(isl_basic_map_is_strict_subset(a.get(), b.get()),
                      "operator <");
}
inline bool operator<(const union_map &a, const union_map &b) {
  return IslBool2Bool(isl_union_map_is_strict_subset(a.get(), b.get()),
                      "operator <");
}

inline bool operator<=(const map &a, const map &b) {
  return IslBool2Bool(isl_map_is_subset(a.get(), b.get()), "operator <=");
}
inline bool operator<=(const basic_map &a, const basic_map &b) {
  return IslBool2Bool(isl_basic_map_is_subset(a.get(), b.get()), "operator <=");
}
inline bool operator<=(const union_map &a, const union_map &b) {
  return IslBool2Bool(isl_union_map_is_subset(a.get(), b.get()), "operator <=");
}

}  // namespace isl
}  // namespace polly