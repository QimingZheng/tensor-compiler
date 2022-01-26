/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-26 10:33:53
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-26 10:52:48
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "auto_scheduler/cost_model.h"
#include "ir/ir_module.h"

namespace polly {
class SearchStrategy {
 public:
  virtual IRModule Search(IRModule module) = 0;
};
}  // namespace polly