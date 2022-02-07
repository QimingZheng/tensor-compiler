/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-26 19:09:13
 * @Last Modified by:   Qiming Zheng
 * @Last Modified time: 2022-01-26 19:09:13
 * @CopyRight: Qiming Zheng
 */
#pragma once

#include "common.h"

#include "ir/ir.h"
#include "ir/ir_module.h"
#include "ir/ir_visitor.h"

#include "pass/transform/fission.h"
#include "pass/transform/fussion.h"
#include "pass/transform/normalization.h"
#include "pass/transform/reorder.h"
#include "pass/transform/split.h"
#include "pass/transform/unroll.h"
#include "pass/transform/vectorization.h"

#include "pass/parallelization/loop_parallel.h"

#include "pass/analysis/transform_analysis_pass.h"
#include "pass/analysis/parallelization_analysis_pass.h"

#include "pass/optimization/constant_folding.h"
#include "pass/optimization/dead_code_elimination.h"

namespace polly {

// Check if a certain transform is legal and do the transformation if
// possible.
class Mutator {
 public:
  // Parallelize a program
  static bool Parallelize(IRHandle program);
  static bool Split(IRHandle program, IRHandle loop, int splitFactor);
  static bool Reorder(IRHandle program, IRHandle outter_loop,
                      IRHandle inner_loop);
  static bool Fussion(IRHandle program, IRHandle first_loop,
                      IRHandle second_loop);
  static bool Fission(IRHandle program, IRHandle loop);
  static bool Unroll(IRHandle program);

 private:
  static bool OfSameScope(IRHandle program, IRHandle first_loop,
                          IRHandle second_loop);
  static bool IsFullyNested(IRHandle outter_loop, IRHandle inner_loop);
};

}  // namespace polly
