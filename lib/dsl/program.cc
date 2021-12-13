#include "program.h"

namespace polly {

Program *Program::GetInstance() {
  if (singleton_ == nullptr) {
    // singleton_ = new Program();
    throw std::runtime_error("Please create a program first");
  }
  return singleton_;
}

Program *Program::singleton_ = nullptr;

}