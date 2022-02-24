#include "program.h"

namespace polly {

Program *Program::GetInstance() {
  if (singleton_ == nullptr) {
    throw std::runtime_error("Please create a program first");
  }
  return singleton_;
}

bool Program::SetReorder(const std::string i, const std::string j) {
  module_.CreateReorderSchedule(i, j);
  return true;
}

bool Program::SetFuse(const std::string i, const std::string j) {
  module_.CreateFuseSchedule(i, j);
  return true;
}

bool Program::SetSplit(const std::string i) {
  module_.CreateSplitSchedule(i);
  return true;
}

Program *Program::singleton_ = nullptr;

}  // namespace polly