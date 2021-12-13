#include "ir.h"
#include "ir_visitor.h"

namespace polly {

void IRVisitor::visit(IRNode *expr) {
  assert(expr != nullptr);
  // std::cout << expr->Type() << std::endl;
  switch (expr->Type()) {
    case IRNodeType::ADD:
      this->visitAdd(static_cast<AddNode *>(expr));
      break;
    case IRNodeType::MUL:
      this->visitMul(static_cast<MulNode *>(expr));
      break;
    case IRNodeType::INT:
      this->visitInt(static_cast<IntNode *>(expr));
      break;
    case IRNodeType::ASSIGN:
      this->visitAssign(static_cast<AssignmentNode *>(expr));
      break;
    case IRNodeType::VAR:
      this->visitVar(static_cast<VarNode *>(expr));
      break;
    case IRNodeType::ACCESS:
      this->visitAccess(static_cast<AccessNode *>(expr));
      break;
    case IRNodeType::TENSOR:
      this->visitTensor(static_cast<TensorNode *>(expr));
      break;
    case IRNodeType::FOR:
      this->visitFor(static_cast<ForNode *>(expr));
      break;

    default:
      throw std::runtime_error("visiting unknonw node");
  }
}

void IRPrinterVisitor::visitInt(IntNode *int_expr) {
  std::cout << int_expr->value;
}
void IRPrinterVisitor::visitAdd(AddNode *add) {
  add->lhs->accept(this);
  std::cout << " + ";
  add->rhs->accept(this);
}
void IRPrinterVisitor::visitMul(MulNode *mul) {
  mul->lhs->accept(this);
  std::cout << " * ";
  mul->rhs->accept(this);
}
void IRPrinterVisitor::visitVar(VarNode *var) { std::cout << var->name; }
void IRPrinterVisitor::visitAccess(AccessNode *access) {
  access->tensor->accept(this);
  std::cout << "(";
  for (int i = 0; i < access->indices.size(); i++) {
    access->indices[i]->accept(this);
    if (i < access->indices.size() - 1) std::cout << ", ";
  }
  std::cout << ")";
}
void IRPrinterVisitor::visitAssign(AssignmentNode *assign) {
  assign->lhs->accept(this);
  std::cout << " = ";
  assign->rhs->accept(this);
  std::cout << "\n";
}
void IRPrinterVisitor::visitTensor(TensorNode *tensor) {
  std::cout << tensor->name;
}
void IRPrinterVisitor::visitFor(ForNode *loop) {
  loop->looping_var_->accept(this);
  std::cout << " {\n";
  for (int i = 0; i < loop->body.size(); i++) {
    loop->body[i]->accept(this);
  }
  std::cout << "}\n";
}

}  // namespace polly