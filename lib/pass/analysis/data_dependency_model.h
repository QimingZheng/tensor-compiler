/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:32:09
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-28 17:06:20
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "polyhedral_model.h"
#include "polyhedral_extraction.h"
#include "solver/solver.h"

namespace polly {

/*!
 * \brief DataDependencyModel models the Write-After-Write, Write-After-Read,
 * Read-After-Write data dependency inside a program.
 *
 * \param ctx The solver context.
 * \param model The extracted polyhedral model.
 */
class DataDependencyModel {
 public:
  DataDependencyModel(solver::context &ctx, PolyhedralModel model);
  /// Add extra constraint that prog-context should be the same across all the
  /// dependency pairs.
  DataDependencyModel(solver::context &ctx, std::vector<int> prog_context,
                      PolyhedralModel model);

  static solver::union_map CreateTransformMap(solver::context &ctx,
                                              PolyhedralModel origin,
                                              PolyhedralModel transformed);

  solver::DependencyMap WAW;
  solver::DependencyMap WAR;
  solver::DependencyMap RAW;

  int GetDepth() { return depth; }

 private:
  solver::AccessMap BuildAccessMap(solver::context &ctx, Statement &st,
                                   ArrayAccess &access);

  PolyhedralModel model;
  int depth = -1;

  // solver::context ctx;
};

}  // namespace polly