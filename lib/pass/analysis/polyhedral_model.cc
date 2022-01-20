#include "polyhedral_model.h"

namespace polly {

IRHandle ReorderedBounds::IslConstraintToBound(solver::constraint c,
                                               std::string loop_name) {
  bool is_negative;
  QuasiAffineExpr expr;
  expr.constant = c.get_constant().integer();
  for (int i = 0; i < c.get_dim(isl_dim_set); i++) {
    if (c.get_coefficient(isl_dim_set, i).integer() != 0) {
      auto dim_name = c.get_dim_name(isl_dim_set, i);
      if (dim_name == loop_name) {
        is_negative = c.get_coefficient(isl_dim_set, i).integer() < 0;
        continue;
      }
      for (int t = 0; t < loop_vars.size(); t++) {
        if (loop_vars[t].as<VarNode>()->id == dim_name) {
          expr.coeffs[dim_name] = c.get_coefficient(isl_dim_set, i).integer();
          break;
        }
      }
    }
  }
  if (!is_negative) {
    for (auto &it : expr.coeffs) {
      it.second = -it.second;
    }
  } else {
    expr.constant += 1;
  }
  return QuasiAffineExprToIR(expr);
}

IRHandle ReorderedBounds::QuasiAffineExprToIR(QuasiAffineExpr expr) {
  std::vector<IRHandle> muls;
  for (auto it : expr.coeffs) {
    for (int i = 0; i < loop_vars.size(); i++) {
      if (loop_vars[i].as<VarNode>()->id == it.first) {
        muls.push_back(MulNode::make(loop_vars[i], IntNode::make(it.second)));
        break;
      }
    }
  }
  muls.push_back(IntNode::make(expr.constant));
  while (muls.size() > 1) {
    IRHandle lhs = muls.back();
    muls.pop_back();
    IRHandle rhs = muls.back();
    muls.pop_back();
    IRHandle add = AddNode::make(lhs, rhs);
    muls.push_back(add);
  }
  return muls[0];
}

void ReorderedBounds::GetReorderedBound(std::vector<IRHandle> loop_vars_,
                                        std::vector<Iteration> extractedIters_,
                                        int ith, int jth) {
  assert(ith != jth);
  loop_vars = loop_vars_;
  extractedIters = extractedIters_;

  solver::context ctx;
  std::vector<std::string> iter_names;
  for (int i = 0; i < loop_vars.size(); i++) {
    iter_names.push_back(loop_vars[i].as<VarNode>()->id);
  }
  solver::IterSet polyhedral(ctx, "S", iter_names);

  for (int i = 0; i < extractedIters.size(); i++) {
    auto coeff = extractedIters[i].lowerBound_.coeffs;
    for (auto &it : coeff) {
      it.second = -it.second;
    }
    coeff[extractedIters[i].iterName_] = 1;
    auto lb_c = polyhedral.CreateInequality(
        coeff, extractedIters[i].lowerBound_.constant);
    polyhedral.add_constraint(lb_c);
    coeff = extractedIters[i].upperBound_.coeffs;
    coeff[extractedIters[i].iterName_] = -1;
    auto ub_c = polyhedral.CreateInequality(
        coeff, extractedIters[i].upperBound_.constant);
    polyhedral.add_constraint(ub_c);
  }

  std::swap(loop_vars[ith], loop_vars[jth]);

  for (int i = loop_vars.size() - 1; i >= 0; i--) {
    auto ubs = polyhedral.GetUpperBounds(loop_vars[i].as<VarNode>()->id);
    auto lbs = polyhedral.GetLowerBounds(loop_vars[i].as<VarNode>()->id);

    assert(ubs.size() == 1);
    assert(lbs.size() == 1);

    loop_vars[i].as<VarNode>()->max =
        IslConstraintToBound(ubs[0], loop_vars[i].as<VarNode>()->id);
    loop_vars[i].as<VarNode>()->min =
        IslConstraintToBound(lbs[0], loop_vars[i].as<VarNode>()->id);
    loop_vars[i].as<VarNode>()->increment = IntNode::make(1);

    polyhedral = polyhedral.project_onto(loop_vars[i].as<VarNode>()->id);
  }
}

}  // namespace polly