/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:31:28
 * @Last Modified by:   Qiming Zheng
 * @Last Modified time: 2022-01-18 20:31:28
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

namespace polly {

// Find Strongly Connected Components in a program, then break the program
// according to each statement's connection component (using Fission/Fussion).
class SyncParallel {
 public:
  SyncParallel() {}
};

}  // namespace polly