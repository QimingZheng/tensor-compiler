/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:30:14
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-28 16:59:40
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "isl/common.h"
#include "isl/space.h"
#include "isl/set.h"
#include "isl/map.h"
#include "isl/context.h"

namespace polly {

namespace solver {

using namespace polly::isl;

/// A polyhedral that defines the iteration domain.
// TODO: transform iteration-domain to schedule map.
class IterSet {
  IterSet(basic_set &s, space &sp, std::map<std::string, int> nestings,
          std::string name)
      : domain_(s), spc(sp), nestings(nestings), name(name) {
    domain_.set_name(name);
  }

 public:
  IterSet(const IterSet &other) {
    domain_ = other.domain_.copy();
    spc = other.spc.copy();
    name = other.name;
    nestings = other.nestings;
  }

  // name: point set identifier.
  // dim_names: name of each dimensions.
  IterSet(const context &ctx, const std::string &name,
          std::vector<std::string> dim_names)
      : name(name) {
    for (int i = 0; i < dim_names.size(); i++) {
      nestings[dim_names[i]] = i;
    }
    spc = space(ctx, set_tuple(dim_names));
    spc.set_name(space::dimension_type::variable, name);
    domain_ = basic_set::universe(spc);
  }

  void add_constraint(constraint c) {
    domain_.add_constraint(c);
    domain_.remove_redundancies();
  }

  constraint CreateEquality(std::map<std::string, int> coeffs,
                            int constant = 0);
  constraint CreateInequality(std::map<std::string, int> coeffs,
                              int constant = 0);

  IterSet project_onto(std::string i) {
    basic_set b =
        isl_basic_set_project_out(domain_.copy(), isl_dim_set, nestings[i], 1);
    b.remove_redundancies();
    std::map<std::string, int> nest;
    int order = nestings[i];
    nestings.erase(i);
    for (auto i : nestings) {
      if (i.second > order) {
        nest[i.first] = i.second - 1;
      } else {
        nest[i.first] = i.second;
      }
    }
    return IterSet(b, spc, nest, name);
  }
  constraint_list GetBounds(std::string loop);

  std::vector<constraint> GetUpperBounds(std::string loop);

  std::vector<constraint> GetLowerBounds(std::string loop);

  void reorder(std::string i, std::string j) {
    std::swap(nestings[i], nestings[j]);
  }
  std::vector<std::string> getDims() {
    std::vector<std::string> ret(nestings.size());
    for (auto i : nestings) {
      ret[i.second] = i.first;
    }
    return ret;
  }

  void dump() { domain_.dump(); }

  bool isEmpty() { return domain_.is_empty(); 
  }

  size_t get_size() { return domain_.get_size(); }

  friend class AccessMap;

 private:
  basic_set domain_;
  space spc;
  std::string name;
  std::map<std::string, int> nestings;
};

/// Modeling the array accesses.
class AccessMap {
 public:
  AccessMap(context ctx, std::string statement_name,
            std::vector<std::string> iter_name, std::string array_name,
            std::vector<std::string> indices_name)
      : array_name_(array_name), statement_name_(statement_name) {
    spc = space(ctx, input_tuple(iter_name), output_tuple(indices_name));
    spc.set_name(space::dimension_type::output, array_name_);
    spc.set_name(space::dimension_type::input, statement_name);
    array_domain_ = basic_map::universe(spc);
    for (int i = 0; i < iter_name.size(); i++) {
      iters[iter_name[i]] = i;
    }
    for (int i = 0; i < indices_name.size(); i++) {
      indices[indices_name[i]] = i;
    }
  }

  void add_constraint(constraint c) { array_domain_.add_constraint(c); }

  constraint CreateEquality(std::map<std::string, int> coeffs,
                            int constant = 0);

  constraint CreateInequality(std::map<std::string, int> coeffs,
                              int constant = 0);

  space spc;
  std::string statement_name_;
  std::string array_name_;
  basic_map array_domain_;
  std::map<std::string, int> iters;
  std::map<std::string, int> indices;
};

/// Maps an statement iteration instance to the logical time space.
class ScheduleMap {
 public:
  ScheduleMap(context &ctx, std::string statement_name,
              std::vector<std::string> iter_name, std::vector<int> prog_context,
              int schedule_dim);

  static union_map PreceedMap(context &ctx, int schedule_dim);

  static union_map OneOneMap(context &ctx, std::vector<std::string> ori_iter,
                             std::vector<int> ori_prog,
                             std::vector<std::string> tr_iter,
                             std::vector<int> tr_prog, int schedule_dim);

  static union_map ParallelMap(context &ctx, std::vector<std::string> iter,
                               std::vector<int> prog, int schedule_dim);

  static union_map ConcurrentMap(context &ctx, int pos, int i, int j,
                                 int schedule_dim);

  union_map schedule;
};

/// Modeling the dependency exists between two groups of array-access.
/// i.e. Given: groupA -> X[], groupB -> X[];
class DependencyMap {
 public:
  DependencyMap() {}
  DependencyMap(context &ctx, std::vector<AccessMap> groupA,
                std::vector<AccessMap> groupB,
                std::vector<std::vector<std::string>> groupAIters,
                std::vector<std::vector<std::string>> groupBIters,
                std::vector<std::vector<int>> groupAProgs,
                std::vector<std::vector<int>> groupBProgs, int schedule_dim)
      : groupA(groupA), groupB(groupB) {
    union_map A(ctx, "{}");
    for (int i = 0; i < groupA.size(); i++) {
      A = A | union_map(groupA[i].array_domain_);
    }
    union_map B(ctx, "{}");
    for (int i = 0; i < groupB.size(); i++) {
      B = B | union_map(groupB[i].array_domain_);
    }
    auto dependency_flow = A(B ^ (-1));
    union_map schedule(ctx, "{}");
    for (int i = 0; i < groupA.size(); i++) {
      schedule =
          schedule | ScheduleMap(ctx, groupA[i].statement_name_, groupAIters[i],
                                 groupAProgs[i], schedule_dim)
                         .schedule;
    }
    for (int i = 0; i < groupB.size(); i++) {
      schedule =
          schedule | ScheduleMap(ctx, groupB[i].statement_name_, groupBIters[i],
                                 groupBProgs[i], schedule_dim)
                         .schedule;
    }
    dependency_flow = (((dependency_flow ^ (-1))(schedule)) ^ (-1))(schedule);
    dependency_flow =
        dependency_flow & solver::ScheduleMap::PreceedMap(ctx, schedule_dim);
    dependency = dependency_flow;
  }

  DependencyMap(context &ctx, std::vector<int> progs, int schedule_dim,
                union_map &dep) {
    dependency = dep;

    union_map prog_equ = union_map(ctx, "{}");

    // space spc = dependency.get_space();

    space spc(ctx, input_tuple(schedule_dim), output_tuple(schedule_dim));
    basic_map subPreceeds = basic_map::universe(spc);
    for (int j = 0; j < 2 * progs.size(); j++) {
      constraint c = constraint::equality(spc);
      value val(ctx, 1);
      c.set_coefficient(space::input, j, val);
      val = value(ctx, -1);
      c.set_coefficient(space::output, j, val);
      subPreceeds.add_constraint(c);
    }

    prog_equ = prog_equ | union_map(subPreceeds);

    dependency = dependency & prog_equ;
  }

  union_map dependency;

 private:
  std::vector<AccessMap> groupA;
  std::vector<AccessMap> groupB;
};

class Solver {
 public:
  Solver() {}

  /// \sum_{i} k_i x_i + c = 0

  /// \sum_{i} k_i x_i + c \>= 0
};

}  // namespace solver

}  // namespace polly