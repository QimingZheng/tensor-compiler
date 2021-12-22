#include "expr.h"
#include "program.h"

namespace polly {

Assignment::Assignment(const Expr lhs, const Expr &rhs) {
  handle_ = AssignmentNode::make(lhs.GetIRHandle(), rhs.GetIRHandle());
  Program::GetInstance()->AddStmt(this);
}

}  // namespace polly