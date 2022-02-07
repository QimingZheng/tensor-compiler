#include "expr.h"
#include "program.h"

namespace polly {

Variable::Variable(const Expr &min, const Expr &max, const Expr &increament) {
  id = IRNodeKeyGen::GetInstance()->YieldVarKey();
  handle_ = VarNode::make(id, min.GetIRHandle(), max.GetIRHandle(),
                          increament.GetIRHandle());
  Program::GetInstance()->EnterLoop(this);
}

Variable::~Variable() { Program::GetInstance()->ExitLoop(this); }

Expr operator+(const Expr &a, const Expr &b) {
  return Expr(AddNode::make(a.GetIRHandle(), b.GetIRHandle()));
}

Expr operator-(const Expr &a, const Expr &b) {
  return Expr(SubNode::make(a.GetIRHandle(), b.GetIRHandle()));
}
Expr operator*(const Expr &a, const Expr &b) {
  return Expr(MulNode::make(a.GetIRHandle(), b.GetIRHandle()));
}
Expr operator/(const Expr &a, const Expr &b) {
  return Expr(DivNode::make(a.GetIRHandle(), b.GetIRHandle()));
}
Expr operator%(const Expr &a, const Expr &b) {
  return Expr(ModNode::make(a.GetIRHandle(), b.GetIRHandle()));
}

Access::Access(const Expr tensor, const std::vector<Expr> &indices) {
  std::vector<IRHandle> indicesIRNodes;
  indicesIRNodes.clear();
  std::transform(
      indices.begin(), indices.end(), std::back_inserter(indicesIRNodes),
      [](const Expr expr) -> IRHandle { return expr.GetIRHandle(); });
  handle_ = AccessNode::make(tensor.GetIRHandle(), indicesIRNodes);
}

Tensor::Tensor(std::vector<int64_t> shape) : shape(shape) {
  id = IRNodeKeyGen::GetInstance()->YieldTensorKey();
  handle_ = TensorNode::make(id, shape);
  Program::GetInstance()->DeclareTensor(this);
}

Constant::Constant(const std::string name) { handle_ = ConstNode::make(name); }

Max::Max(const Expr &a, const Expr &b) {
  handle_ = MaxNode::make(a.GetIRHandle(), b.GetIRHandle());
}
Min::Min(const Expr &a, const Expr &b) {
  handle_ = MinNode::make(a.GetIRHandle(), b.GetIRHandle());
}

}  // namespace polly