#include "expr.h"
#include "program.h"

namespace polly {

Variable::Variable(const std::string &name, const Expr &min, const Expr &max,
                   const Expr &increament)
    : name(name) {
  handle_ = VarNode::make(name, min.GetIRHandle(), max.GetIRHandle(),
                          increament.GetIRHandle());
  Program::GetInstance()->EnterLoop(this);
}

Variable::~Variable() { Program::GetInstance()->ExitLoop(this); }

Add operator+(const Expr &a, const Expr &b) { return Add(a, b); }

Mul operator*(const Expr &a, const Expr &b) { return Mul(a, b); }

// Sub operator-(const Expr &a, const Expr &b) { return Sub(a, b); }

// Div operator/(const Expr &a, const Expr &b) { return Div(a, b); }

Access::Access(const Expr tensor, const std::vector<Expr> &indices) {
  std::vector<IRHandle> indicesIRNodes;
  indicesIRNodes.clear();
  std::transform(
      indices.begin(), indices.end(), std::back_inserter(indicesIRNodes),
      [](const Expr expr) -> IRHandle { return expr.GetIRHandle(); });
  handle_ = AccessNode::make(tensor.GetIRHandle(), indicesIRNodes);
}

Tensor::Tensor(const std::string &name, std::vector<int64_t> shape)
    : name(name), shape(shape) {
  handle_ = TensorNode::make(name, shape);
  Program::GetInstance()->DeclareTensor(this);
}

Constant::Constant(const std::string name) { handle_ = ConstNode::make(name); }

}  // namespace polly