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
    case IRNodeType::FLOAT:
      this->visitFloat(expr.as<FloatNode>());
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
    case IRNodeType::VALUE:
      this->visitVal(expr.as<ValNode>());
      break;
    case IRNodeType::DECLARATION:
      this->visitDecl(expr.as<DeclNode>());
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

    case IRNodeType::MIN:
      this->visitMin(expr.as<MinNode>());
      break;
    case IRNodeType::MAX:
      this->visitMax(expr.as<MaxNode>());
      break;

    case IRNodeType::VEC:
      this->visitVec(expr.as<VecNode>());
      break;
    case IRNodeType::VEC_SCALAR:
      this->visitVecScalar(expr.as<VecScalarNode>());
      break;
    case IRNodeType::VEC_LOAD:
      this->visitVecLoad(expr.as<VecLoadNode>());
      break;
    case IRNodeType::VEC_BROADCAST_LOAD:
      this->visitVecBroadCastLoad(expr.as<VecBroadCastLoadNode>());
      break;
    case IRNodeType::VEC_STORE:
      this->visitVecStore(expr.as<VecStoreNode>());
      break;
    case IRNodeType::VEC_ADD:
      this->visitVecAdd(expr.as<VecAddNode>());
      break;
    case IRNodeType::VEC_SUB:
      this->visitVecSub(expr.as<VecSubNode>());
      break;
    case IRNodeType::VEC_MUL:
      this->visitVecMul(expr.as<VecMulNode>());
      break;
    case IRNodeType::VEC_DIV:
      this->visitVecDiv(expr.as<VecDivNode>());
      break;

    default:
      std::cout << expr.Type() << '\n';
      throw std::runtime_error("visiting unknonw node");
  }
}

void IRPrinterVisitor::visitInt(IntHandle int_expr) {
  std::cout << int_expr->value;
}
void IRPrinterVisitor::visitFloat(FloatHandle float_expr) {
  std::cout << float_expr->value;
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
void IRPrinterVisitor::visitVar(VarHandle var) { std::cout << var->id; }
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
  std::cout << tensor->id;
}
void IRPrinterVisitor::visitVal(ValHandle val) { std::cout << val->id; }
void IRPrinterVisitor::visitDecl(DeclHandle decl) { decl->decl.accept(this); }
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

void IRPrinterVisitor::visitMin(MinHandle min) {
  std::cout << "min(";
  min->lhs.accept(this);
  std::cout << ", ";
  min->rhs.accept(this);
  std::cout << ")";
}
void IRPrinterVisitor::visitMax(MaxHandle max) {
  std::cout << "max(";
  max->lhs.accept(this);
  std::cout << ", ";
  max->rhs.accept(this);
  std::cout << ")";
}

void IRPrinterVisitor::visitVec(VecHandle vec) { std::cout << vec->id; }
void IRPrinterVisitor::visitVecScalar(VecScalarHandle vecScalar) {
  vec_case(vecScalar->length);
  vecScalar->vec.accept(this);
  std::cout << " = ";
  vec_case(vecScalar->length);

  vecScalar->scalar.accept(this);
  std::cout << ")";
  std::cout << ";\n";
}
void IRPrinterVisitor::visitVecLoad(VecLoadHandle vecLoad) {
  vec_case(vecLoad->length);

  vecLoad->vec.accept(this);
  std::cout << " = ";
  vec_case(vecLoad->length);

  std::cout << "&";
  vecLoad->data.accept(this);
  std::cout << ")";
  std::cout << ";\n";
}
void IRPrinterVisitor::visitVecBroadCastLoad(
    VecBroadCastLoadHandle vecBroadCastLoad) {
  vec_case(vecBroadCastLoad->length);

  vecBroadCastLoad->vec.accept(this);
  std::cout << " = ";
  vec_case(vecBroadCastLoad->length);

  std::cout << "&";
  vecBroadCastLoad->data.accept(this);
  std::cout << ")";
  std::cout << ";\n";
}
void IRPrinterVisitor::visitVecStore(VecStoreHandle vecStore) {
  vec_case(vecStore->length);
  std::cout << "&";
  vecStore->data.accept(this);
  std::cout << ", ";
  vecStore->vec.accept(this);
  std::cout << ")";
  std::cout << ";\n";
}
void IRPrinterVisitor::visitVecAdd(VecAddHandle add) {
  vec_case(add->length);

  add->vec.accept(this);
  std::cout << " = ";
  vec_case(add->length);

  add->lhs.accept(this);
  std::cout << ", ";
  add->rhs.accept(this);
  std::cout << ")";
  std::cout << ";\n";
}
void IRPrinterVisitor::visitVecSub(VecSubHandle sub) {
  vec_case(sub->length);

  sub->vec.accept(this);
  std::cout << " = ";
  vec_case(sub->length);

  sub->lhs.accept(this);
  std::cout << ", ";
  sub->rhs.accept(this);
  std::cout << ")";
  std::cout << ";\n";
}
void IRPrinterVisitor::visitVecMul(VecMulHandle mul) {
  vec_case(mul->length);

  mul->vec.accept(this);
  std::cout << " = ";
  vec_case(mul->length);

  mul->lhs.accept(this);
  std::cout << ", ";
  mul->rhs.accept(this);
  std::cout << ")";
  std::cout << ";\n";
}
void IRPrinterVisitor::visitVecDiv(VecDivHandle div) {
  vec_case(div->length);

  div->vec.accept(this);
  std::cout << " = ";
  vec_case(div->length);

  div->lhs.accept(this);
  std::cout << ", ";
  div->rhs.accept(this);
  std::cout << ")";
  std::cout << ";\n";
}

void IRPrinterVisitor::vec_case(int vecLen) {
  std::cout << "simd" << vecLen << " ";
}

}  // namespace polly