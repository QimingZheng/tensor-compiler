#include "ir.h"
#include "ir_visitor.h"

namespace polly {

template <typename T>
T IRMutatorVisitor::_replace_subnode_helper(T op_node, bool skipReplace) {
  assert(op_node != NullIRHandle);
  // This check is to avoid the case when the replaceTo node has replaceFrom as
  // its descendant. In that case, we will fall into an infinite replacement
  // iterative loop.
  if (op_node.equals(replaceTo)) {
    return op_node;
  }
  op_node.accept(this);
  if (op_node.equals(replaceFrom) && !skipReplace) {
    return replaceTo;
  }
  return op_node;
}

void IRMutatorVisitor::visitInt(IntHandle int_expr) {
  // PASS
}

void IRMutatorVisitor::visitAdd(AddHandle add) {
  assert(add != nullptr);
  add->lhs = _replace_subnode_helper(add->lhs);
  add->rhs = _replace_subnode_helper(add->rhs);
}

void IRMutatorVisitor::visitSub(SubHandle sub) {
  assert(sub != nullptr);
  sub->lhs = _replace_subnode_helper(sub->lhs);
  sub->rhs = _replace_subnode_helper(sub->rhs);
}

void IRMutatorVisitor::visitMul(MulHandle mul) {
  assert(mul != nullptr);
  mul->lhs = _replace_subnode_helper(mul->lhs);
  mul->rhs = _replace_subnode_helper(mul->rhs);
}

void IRMutatorVisitor::visitDiv(DivHandle div) {
  assert(div != nullptr);
  div->lhs = _replace_subnode_helper(div->lhs);
  div->rhs = _replace_subnode_helper(div->rhs);
}

void IRMutatorVisitor::visitMod(ModHandle mod) {
  assert(mod != nullptr);
  mod->lhs = _replace_subnode_helper(mod->lhs);
  mod->rhs = _replace_subnode_helper(mod->rhs);
}

void IRMutatorVisitor::visitVar(VarHandle var) {
  // PASS
}

void IRMutatorVisitor::visitAccess(AccessHandle access) {
  assert(access != nullptr);
  access->tensor = _replace_subnode_helper(access->tensor);
  for (int i = 0; i < access->indices.size(); i++) {
    access->indices[i] = _replace_subnode_helper(access->indices[i]);
  }
}

void IRMutatorVisitor::visitAssign(AssignmentHandle assign) {
  assert(assign != nullptr);
  assign->lhs = _replace_subnode_helper(assign->lhs);
  assign->rhs = _replace_subnode_helper(assign->rhs);
}

void IRMutatorVisitor::visitTensor(TensorHandle tensor) {
  // PASS
}

void IRMutatorVisitor::visitFor(ForHandle loop) {
  assert(loop != nullptr);
  loop->looping_var_ =
      _replace_subnode_helper(loop->looping_var_, inplaceMutation);
  for (int i = 0; i < loop->body.size(); i++) {
    loop->body[i] = _replace_subnode_helper(loop->body[i]);
  }
}

void IRMutatorVisitor::visitConst(ConstHandle con) {
  // Pass
}

void IRMutatorVisitor::visitPrint(PrintHandle print) {
  print->print.accept(this);
}

void IRMutatorVisitor::visitFunc(FuncHandle func) {
  for (int i = 0; i < func->body.size(); i++) {
    func->body[i].accept(this);
  }
}

void IRMutatorVisitor::visitMin(MinHandle min) {
  assert(min != nullptr);
  min->lhs = _replace_subnode_helper(min->lhs);
  min->rhs = _replace_subnode_helper(min->rhs);
}
void IRMutatorVisitor::visitMax(MaxHandle max) {
  assert(max != nullptr);
  max->lhs = _replace_subnode_helper(max->lhs);
  max->rhs = _replace_subnode_helper(max->rhs);
}

}  // namespace polly
