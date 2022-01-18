#pragma once

#include "common.h"
#include "space_util.h"

namespace polly {

namespace isl {

class expression;
class space;

class identifier {
 public:
  identifier() : m_data(nullptr) {}
  identifier(const std::string &name) : m_name(name), m_data(nullptr) {}
  identifier(void *data) : m_data(data) {}
  identifier(const std::string &name, void *data)
      : m_name(name), m_data(data) {}
  identifier(isl_id *c_id)
      : m_name(isl_id_get_name(c_id)), m_data(isl_id_get_user(c_id)) {}
  isl_id *c_id(isl_ctx *c_ctx) const {
    const char *c_name = m_name.empty() ? nullptr : m_name.c_str();
    if (!c_name && !m_data)
      return nullptr;
    else
      return isl_id_alloc(c_ctx, c_name, m_data);
  }

  bool empty() const { return m_name.empty() && m_data == nullptr; }

  void *data() const { return m_data; }
  const std::string &name() const { return m_name; }

 private:
  std::string m_name;
  void *m_data;
};

class tuple {
 public:
  tuple() {}
  tuple(int size) : elements(size) {}
  tuple(const std::string &name, int size) : id(name), elements(size) {}
  tuple(const identifier &id, int size) : id(id), elements(size) {}
  tuple(const std::vector<std::string> &elem_names) {
    for (const auto &name : elem_names) elements.push_back(identifier(name));
  }
  int size() const { return elements.size(); }
  identifier id;
  std::vector<identifier> elements;
};

class parameter_tuple : public tuple {
  using tuple::tuple;
};
class input_tuple : public tuple {
  using tuple::tuple;
};
class output_tuple : public tuple {
  using tuple::tuple;
};
class set_tuple : public tuple {
  using tuple::tuple;
};

class space : public object<isl_space> {
  friend class set;
  friend class map;
  friend class constraint;

 public:
  enum dimension_type {
    parameter = isl_dim_param,
    input = isl_dim_in,
    output = isl_dim_out,
    variable = isl_dim_set,
    div = isl_dim_div,
    constant = isl_dim_cst,
    all = isl_dim_all
  };

  explicit space() : object(nullptr, nullptr) {}
  space(isl_space *ptr) : object(ptr) {}

  space(const context &ctx, const parameter_tuple &params)
      : object(ctx, isl_space_params_alloc(ctx.get(), params.size())) {
    set_identifiers(parameter, params);
  }

  space(const context &ctx, const tuple &params, const tuple &vars)
      : object(ctx,
               isl_space_set_alloc(ctx.get(), params.size(), vars.size())) {
    set_identifiers(parameter, params);
    set_identifiers(variable, vars);
  }

  space(const context &ctx, const set_tuple &vars)
      : object(ctx, isl_space_set_alloc(ctx.get(), 0, vars.size())) {
    set_identifiers(variable, vars);
  }

  space(const context &ctx, const tuple &params, const tuple &in,
        const tuple &out)
      : object(ctx, isl_space_alloc(ctx.get(), params.size(), in.size(),
                                    out.size())) {
    set_identifiers(parameter, params);
    set_identifiers(input, in);
    set_identifiers(output, out);
  }

  space(const context &ctx, const input_tuple &in, const output_tuple &out)
      : object(ctx, isl_space_alloc(ctx.get(), 0, in.size(), out.size())) {
    set_identifiers(input, in);
    set_identifiers(output, out);
  }

  static space for_parameters(const context &ctx, int param_count) {
    isl_space *s = isl_space_params_alloc(ctx.get(), param_count);
    return space(s);
  }

  static space for_set(const context &ctx, int param_count, int dim) {
    isl_space *s = isl_space_set_alloc(ctx.get(), param_count, dim);
    return space(s);
  }

  static space for_map(const context &ctx, int param_count, int in_dim,
                       int out_dim) {
    isl_space *s = isl_space_alloc(ctx.get(), param_count, in_dim, out_dim);
    return space(s);
  }

  static space from(const space &domain, const space &range) {
    return space(
        isl_space_map_from_domain_and_range(domain.copy(), range.copy()));
  }

  isl::context get_context() const { return isl_space_get_ctx(get()); }

  expression operator()(dimension_type type, int index);
  expression param(int index) const;
  expression var(int index) const;
  expression in(int index) const;
  expression out(int index) const;
  expression val(int value) const;

  space range() {
    assert(isl_space_is_map(get()));
    return isl_space_range(copy());
  }

  space domain() {
    assert(isl_space_is_map(get()));
    return isl_space_domain(copy());
  }
  unsigned int dimension(dimension_type type) const {
    return isl_space_dim(get(), (isl_dim_type)type);
  }

  void add_dimensions(space::dimension_type t, unsigned n = 1) {
    m_object = isl_space_add_dims(m_object, (isl_dim_type)t, n);
  }

  void insert_dimensions(dimension_type type, unsigned pos, unsigned n = 1) {
    m_object = isl_space_insert_dims(m_object, (isl_dim_type)type, pos, n);
  }
  void drop_dimensions(dimension_type type, unsigned pos, unsigned n = 1) {
    m_object = isl_space_drop_dims(m_object, (isl_dim_type)type, pos, n);
  }

  identifier id(dimension_type type) const {
    isl_id *c_id = isl_space_get_tuple_id(get(), (isl_dim_type)type);
    identifier id(c_id);
    isl_id_free(c_id);
    return id;
  }
  void set_id(dimension_type type, const identifier &id) {
    isl_id *c_id = id.c_id(m_ctx.get());
    if (c_id)
      m_object = isl_space_set_tuple_id(get(), (isl_dim_type)type, c_id);
  }
  std::string name(dimension_type type) const {
    return isl_space_get_tuple_name(get(), (isl_dim_type)type);
  }
  void set_name(dimension_type type, const std::string &name) {
    m_object =
        isl_space_set_tuple_name(m_object, (isl_dim_type)type, name.c_str());
  }
  void set_name(dimension_type type, unsigned pos, const std::string &name) {
    m_object =
        isl_space_set_dim_name(m_object, (isl_dim_type)type, pos, name.c_str());
  }
  bool is_params() const { return isl_space_is_params(get()); }
  bool is_set() const { return isl_space_is_set(get()); }
  bool is_map() const { return isl_space_is_map(get()); }
  bool is_wrapping() const { return isl_space_is_wrapping(get()); }

  space &wrap() {
    m_object = isl_space_wrap(m_object);
    return *this;
  }

  space wrapped() { return isl_space_wrap(copy()); }

 private:
  space(context &ctx, isl_space *space) : object(ctx, space) {}

  void set_identifiers(dimension_type type, const tuple &tup) {
    if (!tup.size()) return;

    set_id(type, tup.id);

    int dim_idx = 0;
    for (const identifier &elem : tup.elements) {
      if (elem.empty()) continue;

      isl_id *c_id =
          isl_id_alloc(m_ctx.get(), elem.name().c_str(), elem.data());
      m_object = isl_space_set_dim_id(get(), (isl_dim_type)type, dim_idx, c_id);
      ++dim_idx;
    }
  }
};

inline space align_params(const space &lhs, const space &rhs) {
  return isl_space_align_params(lhs.copy(), rhs.copy());
}

inline space product(const space &lhs, const space &rhs) {
  return isl_space_product(lhs.copy(), rhs.copy());
}

inline space domain_product(const space &lhs, const space &rhs) {
  return isl_space_domain_product(lhs.copy(), rhs.copy());
}

inline space range_product(const space &lhs, const space &rhs) {
  return isl_space_range_product(lhs.copy(), rhs.copy());
}

inline space operator*(const space &lhs, const space &rhs) {
  return product(lhs, rhs);
}

class local_space : public object<isl_local_space> {
 public:
  local_space(isl_local_space *ptr) : object(ptr) {}
  local_space(const space &parent)
      : object(parent.ctx(), isl_local_space_from_space(parent.copy())) {}

  static local_space from_domain(const local_space &other) {
    return isl_local_space_from_domain(other.copy());
  }

  local_space domain() const { return isl_local_space_domain(copy()); }

  local_space wrapped() const { return isl_local_space_wrap(copy()); }

  isl::space space() const { return isl_local_space_get_space(get()); }

  int dimension(space::dimension_type type) const {
    return isl_local_space_dim(get(), (isl_dim_type)type);
  }

  bool is_set_space() const { return isl_local_space_is_set(get()); }

  expression operator()(space::dimension_type type, int index);

  // bool is_wrapping() const { return isl_local_space_is_wrapping(get()); }
};

}  // namespace isl
}