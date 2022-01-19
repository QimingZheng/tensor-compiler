/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:32:13
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-19 20:36:47
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "pass/pass.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

#include "polyhedral_extraction.h"
#include "data_dependency_model.h"

namespace polly {

class PolyhedralAnalysisPass : public Pass {
  PolyhedralAnalysisPass(solver::context ctx, IRHandle srcProgram,
                         IRHandle tgtProgram, solver::union_map transformMap)
      : srcProgram(srcProgram),
        tgtProgram(tgtProgram),
        ori_tr_map(transformMap) {
    PolyhedralExtraction extraction(srcProgram);
    PolyhedralModel srcModel = extraction.model;
    extraction = PolyhedralExtraction(tgtProgram);
    PolyhedralModel tgtModel = extraction.model;

    DataDependencyModel srcDependency(ctx, srcModel);
    DataDependencyModel tgtDependency(ctx, tgtModel);
    hasConflicts = hasConflict(srcDependency.RAW.dependency,
                               tgtDependency.RAW.dependency, ori_tr_map) ||
                   hasConflict(srcDependency.WAR.dependency,
                               tgtDependency.WAR.dependency, ori_tr_map) ||
                   hasConflict(srcDependency.WAW.dependency,
                               tgtDependency.WAW.dependency, ori_tr_map);
  }

 public:
  bool hasConflicts;
  IRHandle srcProgram;
  IRHandle tgtProgram;
  solver::union_map ori_tr_map;

  static PassRetHandle runPass(PassArgHandle arg) {
    PolyhedralAnalysisPass analysis(
        PassArg::as<Arg>(arg)->ctx, PassArg::as<Arg>(arg)->srcProgram,
        PassArg::as<Arg>(arg)->tgtProgram, PassArg::as<Arg>(arg)->transformMap);
    return Ret::create(analysis.hasConflicts);
  }

  struct Arg : public PassArg {
    IRHandle srcProgram;
    IRHandle tgtProgram;

    solver::context ctx;
    solver::union_map transformMap;

    Arg() {}
    Arg(solver::context ctx, IRHandle p1, IRHandle p2,
        solver::union_map transformMap)
        : ctx(ctx),
          srcProgram(p1),
          tgtProgram(p2),
          transformMap(transformMap) {}
  };
  struct Ret : public PassRet {
    bool hasConflicts;
    static PassRetHandle create(bool hasConflicts) {
      auto ret = std::shared_ptr<Ret>(new Ret);
      ret->hasConflicts = hasConflicts;
      return ret;
    }
  };

  static bool hasConflict(solver::union_map srcMap, solver::union_map tgtMap,
                          solver::union_map transformation) {
    return !((((srcMap ^ (-1))(transformation)) ^ (-1)) - tgtMap).empty();
  }

  // solver::context ctx;
};

}  // namespace polly