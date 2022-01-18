#include "polyhedral_model.h"

namespace polly {

IRHandle ReorderedBounds::IslConstraintToIR(solver::constraint c) {
  QuasiAffineExpr expr;
  expr.constant = c.get_constant().integer();
  for (int i = 0; i < c.get_dim(isl_dim_set); i++) {
    if (c.get_coefficient(isl_dim_set, i).integer() != 0) {
      auto dim_name = c.get_dim_name(isl_dim_set, i);
      for (int t = 0; t < loop_vars.size(); t++) {
        if (loop_vars[t].as<VarNode>()->id == dim_name) {
          expr.coeffs[dim_name] = c.get_coefficient(isl_dim_set, i).integer();
          break;
        }
      }
    }
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
  loop_vars = loop_vars_;
  extractedIters = extractedIters_;

  solver::context ctx;
  std::vector<std::string> iter_names;
  for (int i = 0; i < loop_vars.size(); i++) {
    iter_names.push_back(loop_vars[i].as<VarNode>()->id);
  }
  solver::IterSet polyhedral(ctx, "S", iter_names);

  for (int i = 0; i < extractedIters.size(); i++) {
    auto lb_c =
        polyhedral.CreateInequality(extractedIters[i].lowerBound_.coeffs,
                                    extractedIters[i].lowerBound_.constant);
    polyhedral.add_constraint(lb_c);
    auto ub_c =
        polyhedral.CreateInequality(extractedIters[i].upperBound_.coeffs,
                                    extractedIters[i].upperBound_.constant);
    polyhedral.add_constraint(ub_c);
  }

  std::swap(loop_vars[ith], loop_vars[jth]);

  for (int i = loop_vars.size() - 1; i >= 0; i--) {
    auto ubs = polyhedral.GetUpperBounds(loop_vars[i].as<VarNode>()->id);
    auto lbs = polyhedral.GetLowerBounds(loop_vars[i].as<VarNode>()->id);

    assert(ubs.size() == 1);
    assert(lbs.size() == 1);

    loop_vars[i].as<VarNode>()->max = IslConstraintToIR(ubs[0]);
    loop_vars[i].as<VarNode>()->min = IslConstraintToIR(lbs[0]);
    loop_vars[i].as<VarNode>()->increment = IntNode::make(1);

    polyhedral = polyhedral.project_onto(loop_vars[i].as<VarNode>()->id);
  }
}

}  // namespace polly