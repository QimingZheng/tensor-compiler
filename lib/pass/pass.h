#pragma once

#include "common.h"

namespace polly {

class Pass {
 public:
  enum PassStatus {
    SUCCESS,
    FAIL,
    WAITING,
  };
  Pass() : status_(PassStatus::WAITING) {}

  const PassStatus Status() const { return status_; }

 private:
  PassStatus status_;
};

}  // namespace polly