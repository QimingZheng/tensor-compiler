#include "expr.h"
#include "program.h"

namespace polly {

Variable::Variable(const std::string &name, const Expr &min, const Expr &max,
                   const Expr &increament)
    : name(name) {
  expr_node_ = new VarNode(name, min.GetIRNode(), max.GetIRNode(),
                           increament.GetIRNode());
  Program::GetInstance()->EnterLoop(this);
}

Variable::~Variable() { Program::GetInstance()->ExitLoop(this); }

Add operator+(const Expr &a, const Expr &b) { return Add(a, b); }

Mul operator*(const Expr &a, const Expr &b) { return Mul(a, b); }

Access::Access(const Expr tensor, const std::vector<Expr> &indices) {
  std::vector<IRNode *> indicesIRNodes;
  indicesIRNodes.clear();
  std::transform(indices.begin(), indices.end(),
                 std::back_inserter(indicesIRNodes),
                 [](const Expr expr) -> IRNode * { return expr.GetIRNode(); });
  expr_node_ = new AccessNode(tensor.GetIRNode(), indicesIRNodes);
}

}  // namespace polly