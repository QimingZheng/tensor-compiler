/*
 * @Description: Polly: A DSL compiler for Tensor Program 
 * @Author: Qiming Zheng 
 * @Date: 2022-01-18 20:32:09 
 * @Last Modified by:   Qiming Zheng 
 * @Last Modified time: 2022-01-18 20:32:09 
 * @CopyRight: Qiming Zheng 
 */

#pragma once

#include "common.h"
#include "polyhedral_model.h"
#include "polyhedral_extraction.h"
#include "solver/solver.h"

namespace polly {

class DataDependencyModel {
 public:
  DataDependencyModel(solver::context &ctx, PolyhedralModel model);

  static solver::union_map CreateTransformMap(solver::context &ctx,
                                              PolyhedralModel origin,
                                              PolyhedralModel transformed);

  solver::DependencyMap WAW;
  solver::DependencyMap WAR;
  solver::DependencyMap RAW;

 private:
  solver::AccessMap BuildAccessMap(solver::context &ctx, Statement &st,
                                   ArrayAccess &access);

  PolyhedralModel model;

  // solver::context ctx;
};

}  // namespace polly