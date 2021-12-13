#include "expr.h"
#include "program.h"

namespace polly {

Assignment::Assignment(const Expr lhs, const Expr &rhs) {
  stmt_node_ = new AssignmentNode(lhs.GetIRNode(), rhs.GetIRNode());
  Program::GetInstance()->AddStmt(this);
}

}