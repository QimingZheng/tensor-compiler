#include "ir.h"
#include "ir_visitor.h"

namespace polly {

template <typename T>
T *IRMutatorVisitor::_replace_subnode_helper(T *op_node, bool skipReplace) {
  assert(op_node != nullptr);
  // This check is to avoid the case when the replaceTo node has replaceFrom as
  // its descendant. In that case, we will fall into an infinite replacement
  // iterative loop.
  if (op_node->equals(replaceTo)) {
    return op_node;
  }
  op_node->accept(this);
  if (op_node->equals(replaceFrom) && !skipReplace) {
    return replaceTo;
  }
  return op_node;
}

void IRMutatorVisitor::visitInt(IntNode *int_expr) {
  // PASS
}

void IRMutatorVisitor::visitAdd(AddNode *add) {
  assert(add != nullptr);
  add->lhs = _replace_subnode_helper(add->lhs);
  add->rhs = _replace_subnode_helper(add->rhs);
}

void IRMutatorVisitor::visitSub(SubNode *sub) {
  assert(sub != nullptr);
  sub->lhs = _replace_subnode_helper(sub->lhs);
  sub->rhs = _replace_subnode_helper(sub->rhs);
}

void IRMutatorVisitor::visitMul(MulNode *mul) {
  assert(mul != nullptr);
  mul->lhs = _replace_subnode_helper(mul->lhs);
  mul->rhs = _replace_subnode_helper(mul->rhs);
}

void IRMutatorVisitor::visitDiv(DivNode *div) {
  assert(div != nullptr);
  div->lhs = _replace_subnode_helper(div->lhs);
  div->rhs = _replace_subnode_helper(div->rhs);
}

void IRMutatorVisitor::visitMod(ModNode *mod) {
  assert(mod != nullptr);
  mod->lhs = _replace_subnode_helper(mod->lhs);
  mod->rhs = _replace_subnode_helper(mod->rhs);
}

void IRMutatorVisitor::visitVar(VarNode *var) {
  // PASS
}

void IRMutatorVisitor::visitAccess(AccessNode *access) {
  assert(access != nullptr);
  access->tensor = _replace_subnode_helper(access->tensor);
  for (int i = 0; i < access->indices.size(); i++) {
    access->indices[i] = _replace_subnode_helper(access->indices[i]);
  }
}

void IRMutatorVisitor::visitAssign(AssignmentNode *assign) {
  assert(assign != nullptr);
  assign->lhs = _replace_subnode_helper(assign->lhs);
  assign->rhs = _replace_subnode_helper(assign->rhs);
}

void IRMutatorVisitor::visitTensor(TensorNode *tensor) {
  // PASS
}

void IRMutatorVisitor::visitFor(ForNode *loop) {
  assert(loop != nullptr);
  loop->looping_var_ = _replace_subnode_helper(loop->looping_var_, true);
  for (int i = 0; i < loop->body.size(); i++) {
    loop->body[i] = _replace_subnode_helper(loop->body[i]);
  }
}

void IRMutatorVisitor::visitConst(ConstNode *con) {
  // PASS
}

}  // namespace polly
