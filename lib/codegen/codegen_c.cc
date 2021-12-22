#include "codegen.h"

namespace polly {

void CodeGenC::visitInt(IntHandle int_expr) { oss << int_expr->value; }

void CodeGenC::visitAdd(AddHandle add) {
  add->lhs.accept(this);
  oss << " + ";
  add->rhs.accept(this);
}

void CodeGenC::visitSub(SubHandle sub) {
  sub->lhs.accept(this);
  oss << " + ";
  sub->rhs.accept(this);
}

void CodeGenC::visitMul(MulHandle mul) {
  oss << "(";
  mul->lhs.accept(this);
  oss << ")";
  oss << " * ";
  oss << "(";
  mul->rhs.accept(this);
  oss << ")";
}

void CodeGenC::visitDiv(DivHandle div) {
  oss << "(";
  div->lhs.accept(this);
  oss << ")";
  oss << " / ";
  oss << "(";
  div->rhs.accept(this);
  oss << ")";
}

void CodeGenC::visitMod(ModHandle mod) {
  oss << "(";
  mod->lhs.accept(this);
  oss << ")";
  oss << " % ";
  oss << "(";
  mod->rhs.accept(this);
  oss << ")";
}

void CodeGenC::visitVar(VarHandle var) { oss << var->name; }

void CodeGenC::visitAccess(AccessHandle access) {
  access->tensor.accept(this);
  oss << "[";
  for (int i = 0; i < access->indices.size(); i++) {
    access->indices[i].accept(this);
    oss << "]";
    if (i != access->indices.size() - 1) oss << "[";
  }
}

void CodeGenC::visitAssign(AssignmentHandle assign) {
  oss << getIndent();
  assign->lhs.accept(this);
  oss << " = ";
  assign->rhs.accept(this);
  oss << ";\n";
}

void CodeGenC::visitTensor(TensorHandle tensor) { oss << tensor->name; }

void CodeGenC::visitFor(ForHandle loop) {
  VarHandle loop_var = loop->looping_var_.as<VarNode>();
  oss << getIndent();
  oss << "for (int ";
  loop->looping_var_.accept(this);
  oss << " = ";
  loop_var->min.accept(this);
  oss << ";";
  loop->looping_var_.accept(this);
  oss << " <= ";
  loop_var->max.accept(this);
  oss << ";";
  loop->looping_var_.accept(this);
  oss << " += ";
  loop_var->increment.accept(this);
  oss << ") {\n";
  indent += 1;
  for (int i = 0; i < loop->body.size(); i++) {
    loop->body[i].accept(this);
  }
  indent -= 1;
  oss << getIndent();
  oss << "}\n";
}
void CodeGenC::genCode(IRHandle program, std::vector<IRHandle> &tensors) {
  oss << C_Heaader;
  for (int i = 0; i < tensors.size(); i++) {
    oss << "float " << tensors[i].as<TensorNode>()->name;
    for (int j = 0; j < tensors[i].as<TensorNode>()->shape.size(); j++)
      oss << "[" << tensors[i].as<TensorNode>()->shape[j] << "]";
    oss << ";\n";
  }
  oss << "int main() {\n";
  visit(program);
  oss << "}\n";
}

void CodeGenC::visitConst(ConstHandle con) { oss << con->name; }

}  // namespace polly
