#pragma once

#include "common.h"

namespace polly {

class ArchSpec {
 public:
  enum ArchType {
    CPU,
    NVIDIA_GPU,
  };
  ArchType type_;
  ArchSpec(ArchType type = ArchType::CPU) : type_(type) {}
};
}  // namespace polly