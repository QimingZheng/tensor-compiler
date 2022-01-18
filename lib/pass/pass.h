/*
 * @Description: Polly: A DSL compiler for Tensor Program 
 * @Author: Qiming Zheng 
 * @Date: 2022-01-18 20:30:24 
 * @Last Modified by:   Qiming Zheng 
 * @Last Modified time: 2022-01-18 20:30:24 
 * @CopyRight: Qiming Zheng 
 */
#pragma once

#include "common.h"

namespace polly {

typedef int PassKey;

constexpr PassKey FussionPassID = 1;
constexpr PassKey FissionPassID = 2;
constexpr PassKey UnrollPassID = 3;
constexpr PassKey LoopVectorizationPassID = 4;
constexpr PassKey ConstantFoldingPassID = 5;

struct PassArg;
typedef std::shared_ptr<PassArg> PassArgHandle;
struct PassArg {
  template <typename T>
  static std::shared_ptr<T> as(PassArgHandle arg) {
    return std::static_pointer_cast<T>(arg);
  }
};

struct PassRet;
typedef std::shared_ptr<PassRet> PassRetHandle;
struct PassRet {
  template <typename T>
  static std::shared_ptr<T> as(PassRetHandle arg) {
    return std::static_pointer_cast<T>(arg);
  }
};

class Pass {
 public:
  enum PassStatus {
    VALID,
    INVALID,
  };

  Pass() : status_(PassStatus::INVALID) {}

  virtual PassRetHandle runPass(PassArgHandle) = 0;

  const PassStatus Status() const { return status_; }
  void SetStatus(PassStatus status) { status_ = status; }

 private:
  PassStatus status_;
};

}  // namespace polly