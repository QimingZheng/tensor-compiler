#pragma once

#include "common.h"

namespace polly {

namespace isl {

bool IslBool2Bool(isl_bool boolean, std::string error_msg);
}
}