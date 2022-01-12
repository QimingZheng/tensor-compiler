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
                         std::map<std::string, int> nestings) {
  std::vector<std::string> iter_name;
  std::vector<std::string> date;
  for (auto it : nestings) {
    iter_name.push_back(it.first);
    date.push_back("t" + it.first);
  }

  space spc(ctx, input_tuple(iter_name), output_tuple(date));
  spc.set_name(space::dimension_type::input, statement_name);
  basic_map sch = basic_map::universe(spc);

  for (int t = 0; t < iter_name.size(); t++) {
    constraint c = constraint::equality(sch.get_space());
    value val(ctx, 1);
    c.set_coefficient(space::input, nestings[iter_name[t]], val);
    val = value(ctx, -1);
    c.set_coefficient(space::output, nestings[iter_name[t]], val);
    sch.add_constraint(c);
  }
  schedule = union_map(sch);
}

}  // namespace solver

}  // namespace polly