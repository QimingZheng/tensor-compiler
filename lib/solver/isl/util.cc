#include "util.h"

namespace polly {

namespace isl {
bool IslBool2Bool(isl_bool boolean, std::string error_msg) {
  if (boolean == isl_bool_error) throw std::runtime_error(error_msg);
  if (boolean == isl_bool_false) return false;
  if (boolean == isl_bool_true) return true;
}  // namespace isl
}  // namespace isl
}