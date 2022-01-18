/*
 * @Description: Polly: A DSL compiler for Tensor Program 
 * @Author: Qiming Zheng 
 * @Date: 2022-01-18 20:30:29 
 * @Last Modified by:   Qiming Zheng 
 * @Last Modified time: 2022-01-18 20:30:29 
 * @CopyRight: Qiming Zheng 
 */
#pragma once

#include "common.h"
#include "pass/pass.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "ir/ir_module.h"

namespace polly {

class LoopVectorization : public Pass, public IRVisitor {
 public:
  constexpr static PassKey id = LoopVectorizationPassID;
  LoopVectorization() {}
};

}  // namespace polly
