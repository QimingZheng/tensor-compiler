#pragma once

#include "common.h"
#include "context.h"
#include "object.h"
#include "set_util.h"
#include "util.h"
#include "space.h"

namespace polly {

namespace isl {

class constraint;
class map;
class union_map;
class basic_map;

class basic_set : public object<isl_basic_set> {
 public:
  explicit basic_set() : object(nullptr, nullptr) {}
  basic_set(isl_basic_set *ptr) : object(ptr) {}
  basic_set(const space &spc)
      : object(spc.ctx(), isl_basic_set_empty(spc.copy())) {}
  basic_set(context &ctx, const std::string &text)
      : object(ctx, isl_basic_set_read_from_str(ctx.get(), text.c_str())) {}
  bool is_empty() {
    return IslBool2Bool(isl_basic_set_is_empty(m_object), "empty");
  }
  basic_map operator*(basic_set s);
  basic_map unwrap();
  space get_space() { return space(isl_basic_set_get_space(get())); }
  static basic_set universe(const space &s) {
    return isl_basic_set_universe(s.copy());
  }
  void set_name(std::string name) {
    m_object = isl_basic_set_set_tuple_name(m_object, name.c_str());
  }
  void add_constraint(const constraint &c);
};

class set : public object<isl_set> {
 public:
  set(isl_set *ptr) : object(ptr) {}
  set(context &ctx, const std::string &text)
      : object(ctx, isl_set_read_from_str(ctx.get(), text.c_str())) {}
  bool is_empty() { return IslBool2Bool(isl_set_is_empty(m_object), "empty"); }
  bool is_plain_universe() const { return isl_set_plain_is_universe(get()); }
  map operator*(set s);
  map identity();
  map unwrap();
  set cross(set s) { return isl_set_product(copy(), s.copy()); }
  space get_space() { return space(isl_set_get_space(get())); }
  static set universe(const space &s) { return isl_set_universe(s.copy()); }

  void add_constraint(const constraint &c);
};

class union_set : public object<isl_union_set> {
 public:
  union_set(isl_union_set *ptr) : object(ptr) {}
  union_set(context &ctx, const std::string &text)
      : object(ctx, isl_union_set_read_from_str(ctx.get(), text.c_str())) {}
  union_set(const set &s) : object(s.ctx(), isl_union_set_from_set(s.copy())) {}
  bool is_empty() {
    return IslBool2Bool(isl_union_set_is_empty(m_object), "empty");
  }
  union_map operator*(union_set s);
  union_map identity();
  union_map unwrap();
  union_set cross(union_set s) {
    return isl_union_set_product(copy(), s.copy());
  }
  space get_space() { return space(isl_union_set_get_space(get())); }
};

inline set operator&(const set &a, const set &b) {
  isl_set *res = isl_set_intersect(a.copy(), b.copy());
  return set(res);
}

inline basic_set operator&(const basic_set &a, const basic_set &b) {
  isl_basic_set *res = isl_basic_set_intersect(a.copy(), b.copy());
  return basic_set(res);
}

inline union_set operator&(const union_set &a, const union_set &b) {
  isl_union_set *res = isl_union_set_intersect(a.copy(), b.copy());
  return union_set(res);
}

inline set operator|(const set &a, const set &b) {
  isl_set *res = isl_set_union(a.copy(), b.copy());
  return set(res);
}

inline set operator|(const basic_set &a, const basic_set &b) {
  isl_set *res = isl_basic_set_union(a.copy(), b.copy());
  return set(res);
}

inline union_set operator|(const union_set &a, const union_set &b) {
  isl_union_set *res = isl_union_set_union(a.copy(), b.copy());
  return union_set(res);
}

inline set operator-(const set &a, const set &b) {
  isl_set *res = isl_set_subtract(a.copy(), b.copy());
  return set(res);
}

inline union_set operator-(const union_set &a, const union_set &b) {
  isl_union_set *res = isl_union_set_subtract(a.copy(), b.copy());
  return union_set(res);
}

inline bool operator==(const set &a, const set &b) {
  auto res = isl_set_is_equal(a.get(), b.get());
  return IslBool2Bool(res, "operator ==");
}

inline bool operator==(const basic_set &a, const basic_set &b) {
  auto res = isl_basic_set_is_equal(a.get(), b.get());
  return IslBool2Bool(res, "operator ==");
}

inline bool operator==(const union_set &a, const union_set &b) {
  auto res = isl_union_set_is_equal(a.get(), b.get());
  return IslBool2Bool(res, "operator ==");
}

inline bool operator<=(const set &a, const set &b) {
  auto res = isl_set_is_subset(a.get(), b.get());
  return IslBool2Bool(res, "operator ==");
}

inline bool operator<=(const basic_set &a, const basic_set &b) {
  auto res = isl_basic_set_is_subset(a.get(), b.get());
  return IslBool2Bool(res, "operator <=");
}

inline bool operator<=(const union_set &a, const union_set &b) {
  auto res = isl_union_set_is_subset(a.get(), b.get());
  return IslBool2Bool(res, "operator <=");
}

inline bool operator>=(const set &a, const set &b) { return b <= a; }

inline bool operator>=(const basic_set &a, const basic_set &b) {
  return b <= a;
}

inline bool operator>=(const union_set &a, const union_set &b) {
  return b <= a;
}

inline bool operator<(const set &a, const set &b) {
  isl_bool res = isl_set_is_strict_subset(a.get(), b.get());
  return IslBool2Bool(res, "operator <");
}

inline bool operator<(const union_set &a, const union_set &b) {
  isl_bool res = isl_union_set_is_strict_subset(a.get(), b.get());
  return IslBool2Bool(res, "operator <");
}

inline bool operator>(const set &a, const set &b) { return b < a; }

inline bool operator>(const union_set &a, const union_set &b) { return b < a; }

}  // namespace isl
}