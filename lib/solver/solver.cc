#include "solver.h"

namespace polly {

namespace solver {

constraint IterSet::CreateEquality(std::map<std::string, int> coeffs,
                                   int constant) {
  constraint ret = constraint::equality(spc);
  for (auto it : coeffs) {
    value coeff(domain_.ctx(), it.second);
    ret.set_coefficient(space::variable, nestings[it.first], coeff);
  }
  ret.set_constant(constant);
  return ret;
}

constraint IterSet::CreateInequality(std::map<std::string, int> coeffs,
                                     int constant) {
  constraint ret = constraint::inequality(spc);
  for (auto it : coeffs) {
    value coeff(domain_.ctx(), it.second);
    ret.set_coefficient(space::variable, nestings[it.first], coeff);
  }
  ret.set_constant(constant);
  return ret;
}

constraint_list IterSet::GetBounds(std::string loop) {
  constraint_list lst(domain_);
  int sz = lst.size();
  std::vector<int> to_drop;
  for (int i = 0; i < sz; i++) {
    constraint c = lst.at(i);
    if (c.get_coefficient(isl_dim_set, nestings[loop]).integer() == 0)
      to_drop.push_back(i);
  }
  for (int i = to_drop.size() - 1; i >= 0; i--) {
    lst.drop(to_drop[i]);
  }
  return lst;
}

std::vector<constraint> IterSet::GetUpperBounds(std::string loop) {
  std::vector<constraint> ret;
  constraint_list lst = GetBounds(loop);
  int sz = lst.size();
  for (int i = 0; i < sz; i++) {
    constraint c = lst.at(i);
    if (c.is_equality()) {
      ret.push_back(c);
      continue;
    }
    if (c.get_coefficient(isl_dim_set, nestings[loop]).integer() < 0)
      ret.push_back(c);
  }

  return ret;
}

std::vector<constraint> IterSet::GetLowerBounds(std::string loop) {
  std::vector<constraint> ret;
  constraint_list lst = GetBounds(loop);
  int sz = lst.size();
  for (int i = 0; i < sz; i++) {
    constraint c = lst.at(i);
    if (c.is_equality()) {
      ret.push_back(c);
      continue;
    }

    if (c.get_coefficient(isl_dim_set, nestings[loop]).integer() > 0)
      ret.push_back(c);
  }
  return ret;
}

constraint AccessMap::CreateEquality(std::map<std::string, int> coeffs,
                                     int constant) {
  constraint ret = constraint::equality(spc);
  for (auto it : coeffs) {
    value coeff(array_domain_.ctx(), it.second);
    if (iters.find(it.first) != iters.end())
      ret.set_coefficient(space::input, iters[it.first], coeff);
    else
      ret.set_coefficient(space::output, indices[it.first], coeff);
  }
  ret.set_constant(constant);
  return ret;
}

constraint AccessMap::CreateInequality(std::map<std::string, int> coeffs,
                                       int constant) {
  constraint ret = constraint::inequality(spc);
  for (auto it : coeffs) {
    value coeff(array_domain_.ctx(), it.second);
    if (iters.find(it.first) != iters.end())
      ret.set_coefficient(space::input, iters[it.first], coeff);
    else
      ret.set_coefficient(space::output, indices[it.first], coeff);
  }
  ret.set_constant(constant);

  return ret;
}

ScheduleMap::ScheduleMap(context &ctx, std::string statement_name,
                         std::vector<std::string> iter_name,
                         std::vector<int> prog_context, int schedule_dim) {
  assert(iter_name.size() + 1 == prog_context.size());
  std::vector<std::string> date;
  for (int i = 0; i < schedule_dim; i++) {
    date.push_back("t" + std::to_string(i));
  }

  space spc(ctx, input_tuple(iter_name), output_tuple(date));
  spc.set_name(space::dimension_type::input, statement_name);
  basic_map sch = basic_map::universe(spc);

  for (int t = 0; t < iter_name.size(); t++) {
    constraint c = constraint::equality(sch.get_space());
    value val(ctx, 1);
    c.set_coefficient(space::input, t, val);
    val = value(ctx, -1);
    c.set_coefficient(space::output, 2 * t + 1, val);
    sch.add_constraint(c);
  }
  for (int t = 0; t < prog_context.size(); t++) {
    constraint c = constraint::equality(sch.get_space());
    value val(ctx, prog_context[t]);
    c.set_constant(val);
    val = value(ctx, -1);
    c.set_coefficient(space::output, 2 * t, val);
    sch.add_constraint(c);
  }
  for (int t = 0; t < schedule_dim - iter_name.size() - prog_context.size();
       t++) {
    constraint c = constraint::equality(sch.get_space());
    value val(ctx, 0);
    c.set_constant(val);
    val = value(ctx, 1);
    c.set_coefficient(space::output, iter_name.size() + prog_context.size() + t,
                      val);
    sch.add_constraint(c);
  }
  schedule = union_map(sch);
}

union_map ScheduleMap::PreceedMap(context &ctx, int schedule_dim) {
  union_map preceeds = union_map(ctx, "{}");

  for (int i = 0; i < schedule_dim; i++) {
    space spc(ctx, input_tuple(schedule_dim), output_tuple(schedule_dim));
    basic_map subPreceeds = basic_map::universe(spc);
    for (int j = 0; j < i; j++) {
      constraint c = constraint::equality(spc);
      value val(ctx, 1);
      c.set_coefficient(space::input, j, val);
      val = value(ctx, -1);
      c.set_coefficient(space::output, j, val);
      subPreceeds.add_constraint(c);
    }
    constraint c = constraint::inequality(spc);
    value val(ctx, 1);
    c.set_coefficient(space::input, i, val);
    val = value(ctx, -1);
    c.set_coefficient(space::output, i, val);
    val = value(ctx, -1);
    c.set_constant(val);
    subPreceeds.add_constraint(c);

    preceeds = preceeds | union_map(subPreceeds);
  }
  return preceeds;
}

union_map ScheduleMap::OneOneMap(context &ctx,
                                 std::vector<std::string> ori_iter,
                                 std::vector<int> ori_prog,
                                 std::vector<std::string> tr_iter,
                                 std::vector<int> tr_prog, int schedule_dim) {
  assert(ori_prog.size() == tr_prog.size());
  assert(ori_iter.size() == tr_iter.size());
  assert(ori_iter.size() + 1 == ori_prog.size());

  space spc(ctx, input_tuple(schedule_dim), output_tuple(schedule_dim));
  basic_map ori_tr_map = basic_map::universe(spc);

  for (int i = 0; i < ori_iter.size(); i++) {
    int j = 0;
    for (; j < tr_iter.size(); j++) {
      if (ori_iter[i] == tr_iter[j]) {
        break;
      }
    }
    if (j >= tr_iter.size()) j = i;
    constraint c = constraint::equality(spc);
    value val(ctx, 1);
    c.set_coefficient(space::input, 2 * i + 1, val);
    val = value(ctx, -1);
    c.set_coefficient(space::output, 2 * j + 1, val);
    ori_tr_map.add_constraint(c);
  }

  for (int i = 0; i < ori_prog.size(); i++) {
    constraint c = constraint::equality(spc);
    value val(ctx, -1);
    c.set_coefficient(space::input, 2 * i, val);
    val = value(ctx, ori_prog[i]);
    c.set_constant(val);
    ori_tr_map.add_constraint(c);
  }

  for (int i = 0; i < tr_prog.size(); i++) {
    constraint c = constraint::equality(spc);
    value val(ctx, -1);
    c.set_coefficient(space::output, 2 * i, val);
    val = value(ctx, tr_prog[i]);
    c.set_constant(val);
    ori_tr_map.add_constraint(c);
  }
  for (int i = 0; i < schedule_dim - ori_prog.size() - ori_iter.size(); i++) {
    constraint c = constraint::equality(spc);
    value val(ctx, 1);
    c.set_coefficient(space::input, i + ori_prog.size() + ori_iter.size(), val);
    ori_tr_map.add_constraint(c);
  }
  for (int i = 0; i < schedule_dim - ori_prog.size() - ori_iter.size(); i++) {
    constraint c = constraint::equality(spc);
    value val(ctx, 1);
    c.set_coefficient(space::output, i + ori_prog.size() + ori_iter.size(),
                      val);
    ori_tr_map.add_constraint(c);
  }
  return union_map(ori_tr_map);
}

union_map ScheduleMap::ParallelMap(context &ctx, std::vector<std::string> iter,
                                   std::vector<int> prog, int schedule_dim) {
  space spc(ctx, input_tuple(schedule_dim), output_tuple(schedule_dim));
  basic_map par_map = basic_map::universe(spc);

  for (int i = 0; i < prog.size(); i++) {
    constraint c = constraint::equality(spc);
    value val(ctx, -1);
    c.set_coefficient(space::input, 2 * i, val);
    val = value(ctx, prog[i]);
    c.set_constant(val);
    par_map.add_constraint(c);
  }

  for (int i = 0; i < prog.size(); i++) {
    constraint c = constraint::equality(spc);
    value val(ctx, -1);
    c.set_coefficient(space::output, 2 * i, val);
    val = value(ctx, prog[i]);
    c.set_constant(val);
    par_map.add_constraint(c);
  }

  for (int i = 0; i < iter.size() - 1; i++) {
    constraint c = constraint::equality(spc);
    value val(ctx, -1);
    c.set_coefficient(space::input, 2 * i + 1, val);
    val = value(ctx, 1);
    c.set_coefficient(space::output, 2 * i + 1, val);
    par_map.add_constraint(c);
  }

  constraint c = constraint::inequality(spc);
  value val(ctx, 1);
  c.set_coefficient(space::input, 2 * iter.size() - 1, val);
  val = value(ctx, -1);
  c.set_coefficient(space::output, 2 * iter.size() - 1, val);
  par_map.add_constraint(c);

  return union_map(par_map);
}

}  // namespace solver

}  // namespace polly