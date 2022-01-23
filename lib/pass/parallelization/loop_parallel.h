/*
 * @Description: Polly: A DSL compiler for Tensor Program 
 * @Author: Qiming Zheng 
 * @Date: 2022-01-23 19:17:06 
 * @Last Modified by:   Qiming Zheng 
 * @Last Modified time: 2022-01-23 19:17:06 
 * @CopyRight: Qiming Zheng 
 */

#include "common.h"

namespace polly {

/// Simply check is the outter most loop can be parallelized.
class LoopParallel {
 public:
  LoopParallel() {}
};

}  // namespace polly