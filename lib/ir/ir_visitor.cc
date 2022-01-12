#include "ir.h"
#include "ir_visitor.h"

namespace polly {

void IRVisitor::visit(IRHandle expr) {
  assert(expr != NullIRHandle);
  switch (expr.Type()) {
    case IRNodeType::ADD:
      this->visitAdd(expr.as<AddNode>());
      break;
    case IRNodeType::SUB:
      this->visitSub(expr.as<SubNode>());
      break;
    case IRNodeType::MUL:
      this->visitMul(expr.as<MulNode>());
      break;
    case IRNodeType::DIV:
      this->visitDiv(expr.as<DivNode>());
      break;
    case IRNodeType::MOD:
      this->visitMod(expr.as<ModNode>());
      break;
    case IRNodeType::INT:
      this->visitInt(expr.as<IntNode>());
      break;
    case IRNodeType::ASSIGN:
      this->visitAssign(expr.as<AssignmentNode>());
      break;
    case IRNodeType::VAR:
      this->visitVar(expr.as<VarNode>());
      break;
    case IRNodeType::ACCESS:
      this->visitAccess(expr.as<AccessNode>());
      break;
    case IRNodeType::TENSOR:
      this->visitTensor(expr.as<TensorNode>());
      break;
    case IRNodeType::FOR:
      this->visitFor(expr.as<ForNode>());
      break;
    case IRNodeType::CONST:
      this->visitConst(expr.as<ConstNode>());
      break;
    case IRNodeType::PRINT:
      this->visitPrint(expr.as<PrintNode>());
      break;
    case IRNodeType::FUNC:
      this->visitFunc(expr.as<FuncNode>());
      break;

    default:
      std::cout << expr.Type() << '\n';
      throw std::runtime_error("visiting unknonw node");
  }
}

void IRPrinterVisitor::visitInt(IntHandle int_expr) {
  std::cout << int_expr->value;
}
void IRPrinterVisitor::visitAdd(AddHandle add) {
  add->lhs.accept(this);
  std::cout << " + ";
  add->rhs.accept(this);
}
void IRPrinterVisitor::visitSub(SubHandle sub) {
  sub->lhs.accept(this);
  std::cout << " - ";
  sub->rhs.accept(this);
}
void IRPrinterVisitor::visitMul(MulHandle mul) {
  mul->lhs.accept(this);
  std::cout << " * ";
  mul->rhs.accept(this);
}
void IRPrinterVisitor::visitDiv(DivHandle div) {
  div->lhs.accept(this);
  std::cout << " / ";
  div->rhs.accept(this);
}
void IRPrinterVisitor::visitMod(ModHandle mod) {
  mod->lhs.accept(this);
  std::cout << " % ";
  mod->rhs.accept(this);
}
void IRPrinterVisitor::visitVar(VarHandle var) { std::cout << var->name; }
void IRPrinterVisitor::visitAccess(AccessHandle access) {
  access->tensor.accept(this);
  std::cout << "(";
  for (int i = 0; i < access->indices.size(); i++) {
    access->indices[i].accept(this);
    if (i < access->indices.size() - 1) std::cout << ", ";
  }
  std::cout << ")";
}
void IRPrinterVisitor::visitAssign(AssignmentHandle assign) {
  assign->lhs.accept(this);
  std::cout << " = ";
  assign->rhs.accept(this);
  std::cout << "\n";
}
void IRPrinterVisitor::visitTensor(TensorHandle tensor) {
  std::cout << tensor->name;
}
void IRPrinterVisitor::visitFor(ForHandle loop) {
  loop->looping_var_.accept(this);
  std::cout << " {\n";
  for (int i = 0; i < loop->body.size(); i++) {
    loop->body[i].accept(this);
  }
  std::cout << "}\n";
}

void IRPrinterVisitor::visitConst(ConstHandle con) { std::cout << con->name; }

void IRPrinterVisitor::visitPrint(PrintHandle print) {
  std::cout << "print ";
  print->print.accept(this);
  std::cout << ";\n";
}

void IRPrinterVisitor::visitFunc(FuncHandle func) {
  for (int i = 0; i < func->body.size(); i++) {
    func->body[i].accept(this);
  }
}

}  // namespace polly