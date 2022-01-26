#include "expr.h"
#include "program.h"

namespace polly {

Assignment::Assignment(const Expr lhs, const Expr &rhs) {
  handle_ =
      AssignmentNode::make(IRNodeKeyGen::GetInstance()->YieldStatementKey(),
                           lhs.GetIRHandle(), rhs.GetIRHandle());
  Program::GetInstance()->AddStmt(this);
}

Print::Print(const Expr print) {
  handle_ = PrintNode::make(IRNodeKeyGen::GetInstance()->YieldStatementKey(),
                            print.GetIRHandle());
  Program::GetInstance()->AddStmt(this);
}

}  // namespace polly