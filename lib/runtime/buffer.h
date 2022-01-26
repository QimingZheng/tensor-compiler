/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-24 21:20:19
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-24 21:22:27
 * @CopyRight: Qiming Zheng
 */
#pragma once

#include "common.h"

namespace polly {

template <typename T>
class Buffer {
 public:
  enum Type {
    CPU_MEM,
    GPU_MEM,
  };

  size_t size;
  size_t stride;
  size_t alignment;
  T* data;
};

}  // namespace polly