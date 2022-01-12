#pragma once

#include "common.h"
#include "space.h"
#include "expression_util.h"
#include "set.h"

namespace polly {

namespace isl {

class expression : public object<isl_aff> {
 public:
  expression(isl_aff *ptr) : object(ptr) {}

  static expression variable(const local_space &spc, space::dimension_type type,
                             unsigned index) {
    local_space expr_space(spc);
    // wrap_space(expr_space, type, index);
    return isl_aff_var_on_domain(expr_space.copy(), (isl_dim_type)type, index);
  }

  static expression value(const local_space &spc, const value &val) {
    isl_local_space *ls = spc.copy();
    if (!isl_local_space_is_set(ls)) ls = isl_local_space_wrap(ls);

    // FIXME: use isl_aff_val_on_domain when available
    isl_aff *expr = isl_aff_zero_on_domain(ls);
    expr = isl_aff_set_constant_val(expr, val.copy());
    return expr;
  }

  static expression value(const local_space &spc, int i) {
    isl_local_space *ls = spc.copy();
    if (!isl_local_space_is_set(ls)) ls = isl_local_space_wrap(ls);

    // FIXME: use isl_aff_val_on_domain when available
    isl_aff *expr = isl_aff_zero_on_domain(ls);
    expr = isl_aff_set_constant_si(expr, i);
    return expr;
  }

  space domain_space() const { return isl_aff_get_domain_space(get()); }

  space get_space() const { return isl_aff_get_space(get()); }

  local_space get_local_space() const { return isl_aff_get_local_space(get()); }

  bool is_constant() const { return isl_aff_is_cst(get()); }

  isl::value constant() const { return isl_aff_get_constant_val(get()); }

  isl::value coefficient(space::dimension_type type, int index) const {
    return isl_aff_get_coefficient_val(get(), (isl_dim_type)type, index);
  }

  bool involves(space::dimension_type type, int index) const {
    return isl_aff_involves_dims(get(), (isl_dim_type)type, index, 1);
  }

 private:
  static void wrap_space(local_space &spc, space::dimension_type &type,
                         unsigned &index) {
    // if (spc.space().is_map()) {
    //   switch (type) {
    //     case space::input:
    //       type = space::variable;
    //       break;
    //     case space::output:
    //       type = space::variable;
    //       index = index + spc.dimension(space::input);
    //       break;
    //     default:;
    //   }
    //   spc = spc.wrapped();
    // }
  }
};

class piecewise_expression : public object<isl_pw_aff> {
 public:
  piecewise_expression(isl_pw_aff *p) : object(p) {}

  // FIXME: This is not exact.
  expression plain_continuous() {
    if (piece_count() != 1) return expression(nullptr);

    expression r(nullptr);
    for_each_piece([&](const set &s, const expression &e) {
      if (s.is_plain_universe()) r = e;
    });
    return r;
  }

  int piece_count() const { return isl_pw_aff_n_piece(get()); }

  template <typename F>
  void for_each_piece(F f) {
    isl_pw_aff_foreach_piece(get(), &for_each_piece_helper<F>, &f);
  }

 private:
  template <typename F>
  static isl_stat for_each_piece_helper(isl_set *domain, isl_aff *expr,
                                        void *data) {
    F *f = static_cast<F *>(data);
    (*f)(set(domain), expression(expr));
    return isl_stat_ok;
  }
};

class multi_expression : public object<isl_multi_aff> {
 public:
  multi_expression(isl_multi_aff *p) : object(p) {}

  multi_expression(const expression &e)
      : object(isl_multi_aff_from_aff(e.copy())) {}

  static multi_expression zero(const isl::space &spc, int count) {
    auto in_space = spc;
    if (in_space.is_map()) in_space = in_space.wrapped();

    auto out_space = isl::space(spc.get_context(), set_tuple(count));

    auto expr_space = isl::space::from(in_space, out_space);
    return isl_multi_aff_zero(expr_space.copy());
  }

  static multi_expression identity(const isl::space &spc) {
    auto in_space = spc;
    if (in_space.is_map()) in_space = in_space.wrapped();

    auto n_dim = in_space.dimension(isl::space::variable);
    auto out_space = isl::space(spc.get_context(), set_tuple(n_dim));

    auto expr_space = isl::space::from(in_space, out_space);
    return isl_multi_aff_identity(expr_space.copy());
  }

  isl::space space() const { return isl_multi_aff_get_space(m_object); }

  isl::space domain_space() const {
    return isl_multi_aff_get_domain_space(m_object);
  }

  int size() const { return space().dimension(isl::space::output); }

  void insert_dims(isl::space::dimension_type type, unsigned i, unsigned n) {
    m_object = isl_multi_aff_insert_dims(m_object, (isl_dim_type)type, i, n);
  }

  void drop_dims(isl::space::dimension_type type, unsigned i, unsigned n) {
    m_object = isl_multi_aff_drop_dims(m_object, (isl_dim_type)type, i, n);
  }

  expression at(int i) const { return isl_multi_aff_get_aff(get(), i); }

  void set(int i, const expression &e) {
    m_object = isl_multi_aff_set_aff(m_object, i, e.copy());
  }
};

inline expression operator+(const expression &lhs, const expression &rhs) {
  return isl_aff_add(lhs.copy(), rhs.copy());
}
inline expression operator+(const expression &lhs, int rhs_int) {
  return isl_aff_add_constant_si(lhs.copy(), rhs_int);
}
inline expression operator+(const expression &lhs, const value &rhs_val) {
  return isl_aff_add_constant_val(lhs.copy(), rhs_val.copy());
}
template <typename T>
inline expression operator+(const T &lhs, const expression &rhs) {
  return rhs + lhs;
}

inline expression operator-(const expression &e) {
  return isl_aff_neg(e.copy());
}

inline expression operator-(const expression &lhs, const expression &rhs) {
  return isl_aff_sub(lhs.copy(), rhs.copy());
}
inline expression operator-(const expression &lhs, const value &rhs_val) {
  return isl_aff_add_constant_val(lhs.copy(), isl_val_neg(rhs_val.copy()));
}
inline expression operator-(const expression &lhs, int rhs_int) {
  return isl_aff_add_constant_si(lhs.copy(), -rhs_int);
}
inline expression operator-(int lhs_val, const expression &rhs) {
  isl_aff *neg_rhs = isl_aff_neg(rhs.copy());
  isl_aff *result = isl_aff_add_constant_si(neg_rhs, lhs_val);
  return result;
}

inline expression operator*(const expression &lhs, const value &rhs) {
  return isl_aff_scale_val(lhs.copy(), rhs.copy());
}
inline expression operator*(const value &lhs, const expression &rhs) {
  return rhs * lhs;
}
inline expression operator*(const expression &lhs, int rhs_int) {
  auto rhs_val = value(lhs.ctx(), rhs_int);
  return lhs * rhs_val;
}
inline expression operator*(int lhs_int, const expression &rhs) {
  return rhs * lhs_int;
}

inline expression operator/(const expression &lhs, const value &rhs) {
  return isl_aff_scale_down_val(lhs.copy(), rhs.copy());
}

inline expression operator/(const expression &lhs, int rhs) {
  return lhs / value(lhs.ctx(), rhs);
}

inline expression operator%(const expression &lhs, const value &rhs) {
  return isl_aff_mod_val(lhs.copy(), rhs.copy());
}

inline expression operator%(const expression &lhs, int rhs_int) {
  auto rhs = value(lhs.ctx(), rhs_int);
  return lhs % rhs;
}

inline expression floor(const expression &e) { return isl_aff_floor(e.copy()); }

inline expression space::operator()(dimension_type type, int index) {
  return expression::variable(local_space(*this), type, index);
}

inline expression space::param(int index) const {
  return expression::variable(local_space(*this), isl::space::parameter, index);
}

inline expression space::var(int index) const {
  return expression::variable(local_space(*this), isl::space::variable, index);
}

inline expression space::in(int index) const {
  return expression::variable(local_space(*this), isl::space::input, index);
}

inline expression space::out(int index) const {
  return expression::variable(local_space(*this), isl::space::output, index);
}

inline expression space::val(int v) const {
  return expression::value(local_space(*this), v);
}

inline expression local_space::operator()(space::dimension_type type,
                                          int index) {
  return expression::variable(*this, type, index);
}

}  // namespace isl
}