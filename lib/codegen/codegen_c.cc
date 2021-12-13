#include "codegen.h"

namespace polly {

void CodeGenC::visitInt(IntNode *int_expr) { oss << int_expr->value; }

void CodeGenC::visitAdd(AddNode *add) {
  add->lhs->accept(this);
  oss << " + ";
  add->rhs->accept(this);
}

void CodeGenC::visitMul(MulNode *mul) {
  mul->lhs->accept(this);
  oss << " * ";
  mul->rhs->accept(this);
}

void CodeGenC::visitVar(VarNode *var) { oss << var->name; }

void CodeGenC::visitAccess(AccessNode *access) {
  access->tensor->accept(this);
  oss << "[";
  for (int i = 0; i < access->indices.size(); i++) {
    access->indices[i]->accept(this);
    oss << "]";
    if (i != access->indices.size() - 1) oss << "[";
  }
}

void CodeGenC::visitAssign(AssignmentNode *assign) {
  oss << getIndent();
  assign->lhs->accept(this);
  oss << " = ";
  assign->rhs->accept(this);
  oss << ";\n";
}

void CodeGenC::visitTensor(TensorNode *tensor) { oss << tensor->name; }

void CodeGenC::visitFor(ForNode *loop) {
  VarNode *loop_var = static_cast<VarNode *>(loop->looping_var_);
  oss << getIndent();
  oss << "for (int ";
  loop_var->accept(this);
  oss << " = ";
  loop_var->min->accept(this);
  oss << ";";
  loop_var->accept(this);
  oss << " <= ";
  loop_var->max->accept(this);
  oss << ";";
  loop_var->accept(this);
  oss << " += ";
  loop_var->increment->accept(this);
  oss << ") {\n";
  indent += 1;
  for (int i = 0; i < loop->body.size(); i++) {
    loop->body[i]->accept(this);
  }
  indent -= 1;
  oss << getIndent();
  oss << "}\n";
}

void CodeGenC::genCode(ForNode *program) {
  oss << C_Heaader;
  visit(program);
  oss << "}\n";
}

}  // namespace polly
