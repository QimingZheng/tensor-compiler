#include "ir.h"
#include "ir_visitor.h"

namespace polly {

IRHandle AddNode::make(IRHandle lhs, IRHandle rhs) {
  AddNode *add = new AddNode();
  add->lhs = lhs;
  add->rhs = rhs;
  return IRHandle(add);
}

IRHandle SubNode::make(IRHandle lhs, IRHandle rhs) {
  SubNode *sub = new SubNode();
  sub->lhs = lhs;
  sub->rhs = rhs;
  return IRHandle(sub);
}

IRHandle MulNode::make(IRHandle lhs, IRHandle rhs) {
  MulNode *mul = new MulNode();
  mul->lhs = lhs;
  mul->rhs = rhs;
  return IRHandle(mul);
}

IRHandle DivNode::make(IRHandle lhs, IRHandle rhs) {
  DivNode *div = new DivNode();
  div->lhs = lhs;
  div->rhs = rhs;
  return IRHandle(div);
}

IRHandle ModNode::make(IRHandle lhs, IRHandle rhs) {
  ModNode *div = new ModNode();
  div->lhs = lhs;
  div->rhs = rhs;
  return IRHandle(div);
}

IRHandle VarNode::make(const std::string name, IRHandle min, IRHandle max,
                       IRHandle increment) {
  VarNode *var = new VarNode();
  var->name = name;
  var->min = min;
  var->max = max;
  var->increment = increment;
  return IRHandle(var);
}

IRHandle IntNode::make(int x) {
  IntNode *node = new IntNode(x);
  return IRHandle(node);
}

IRHandle TensorNode::make(const std::string &name,
                          std::vector<int64_t> &shape) {
  TensorNode *tensor = new TensorNode();
  tensor->name = name;
  tensor->shape = shape;
  return IRHandle(tensor);
}

IRHandle AccessNode::make(IRHandle tensor, std::vector<IRHandle> indices) {
  AccessNode *node = new AccessNode;
  node->tensor = tensor;
  node->indices = indices;
  return IRHandle(node);
}

IRHandle AssignmentNode::make(IRHandle lhs, IRHandle rhs) {
  AssignmentNode *node = new AssignmentNode();
  node->lhs = lhs;
  node->rhs = rhs;
  return IRHandle(node);
}

IRHandle ForNode::make(IRHandle looping_var, IRHandle parent_loop) {
  ForNode *node = new ForNode();
  node->looping_var_ = looping_var;
  node->parent_loop_ = parent_loop;
  return IRHandle(node);
}

IRHandle ConstNode::make(std::string name) {
  ConstNode *node = new ConstNode();
  node->name = name;
  return IRHandle(node);
}

void IRHandle::accept(IRVisitor *visitor) { visitor->visit(*this); }

}  // namespace polly