/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:26:38
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-18 20:28:54
 * @CopyRight: Qiming Zheng
 */

#pragma once

#include "common.h"
#include "pass/pass.h"

namespace polly {

class CommonSubExpressionElimication : public Pass {};

}  // namespace polly