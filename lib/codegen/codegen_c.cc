#include "codegen.h"

namespace polly {

std::string CodeGenC::genTensors(std::vector<IRHandle> &tensors) {
  oss.clear();
  for (int i = 0; i < tensors.size(); i++) {
    tensor_name.push_back(tensors[i].as<TensorNode>()->id);
    tensor_shape.push_back(tensors[i].as<TensorNode>()->shape);
    oss << "float " << tensors[i].as<TensorNode>()->id;
    for (int j = 0; j < tensors[i].as<TensorNode>()->shape.size(); j++)
      oss << "[" << tensors[i].as<TensorNode>()->shape[j] << "]";
    oss << ";\n";
  }
  return oss.str();
}

std::string CodeGenC::genTensorParam(std::vector<IRHandle> &tensors) {
  oss.clear();
  for (int i = 0; i < tensors.size(); i++) {
    if (i > 0) oss << ", ";
    oss << tensors[i].as<TensorNode>()->id;
  }
  return oss.str();
}

std::string CodeGenC::genCode(IRHandle program, std::vector<IRHandle> &tensors,
                              std::string program_name) {
  program_ = program;

  oss << C_Heaader;

  oss << "void " << program_name << "(";

  for (int i = 0; i < tensors.size(); i++) {
    if (i > 0) oss << ", ";
    tensor_name.push_back(tensors[i].as<TensorNode>()->id);
    tensor_shape.push_back(tensors[i].as<TensorNode>()->shape);
    oss << "float " << tensors[i].as<TensorNode>()->id;
    for (int j = 0; j < tensors[i].as<TensorNode>()->shape.size(); j++)
      oss << "[" << tensors[i].as<TensorNode>()->shape[j] << "]";
  }

  oss << ") {\n";
  visit(program);
  oss << "}\n";

  return oss.str();
}

std::string CodeGenC::tensor_shape_str(std::vector<int64_t> shape) {
  std::string ret = "";
  for (int i = 0; i < shape.size(); i++) {
    ret += "[" + std::to_string(shape[i]) + "]";
  }
  return ret;
}

void CodeGenC::create_method(std::string method_name,
                             std::vector<std::string> tensor_name,
                             std::vector<std::string> outter_loop_vars,
                             int level, IRHandle loop) {
  std::ostringstream dec;
  dec << "inline void " << method_name << "(";
  for (int i = 0; i < tensor_name.size(); i++) {
    dec << "float " << tensor_name[i] << tensor_shape_str(tensor_shape[i])
        << ", ";
  }
  for (int i = 0; i < outter_loop_vars.size(); i++) {
    dec << "int " << outter_loop_vars[i] << ", ";
  }

  dec << "int worker_size, int wid)";

  oss << dec.str();
  method_decls_.push_back(dec.str() + ";");

  oss << " {\n";
  // TODO: method content here.
  auto loop_var = loop.as<ForNode>()->looping_var_.as<VarNode>();
  oss << getIndent();
  oss << "for (int " << loop_var->id << " = wid; " << loop_var->id << " < ";
  loop_var->max.accept(this);
  oss << "; " << loop_var->id << " += worker_size) {\n";
  indent += 1;
  for (int i = 0; i < loop.as<ForNode>()->body.size(); i++) {
    loop.as<ForNode>()->body[i].accept(this);
  }
  indent -= 1;
  oss << getIndent();
  oss << "}\n";
  oss << "}\n";
}

void CodeGenC::visitInt(IntHandle int_expr) { oss << int_expr->value; }

void CodeGenC::visitFloat(FloatHandle float_expr) {
  oss << "(float)" << float_expr->value;
}

void CodeGenC::visitAdd(AddHandle add) {
  oss << "(";
  add->lhs.accept(this);
  oss << " + ";
  add->rhs.accept(this);
  oss << ")";
}

void CodeGenC::visitSub(SubHandle sub) {
  oss << "(";
  sub->lhs.accept(this);
  oss << " - ";
  sub->rhs.accept(this);
  oss << ")";
}

void CodeGenC::visitMul(MulHandle mul) {
  oss << "(";
  mul->lhs.accept(this);
  // oss << ")";
  oss << " * ";
  // oss << "(";
  mul->rhs.accept(this);
  oss << ")";
}

void CodeGenC::visitDiv(DivHandle div) {
  oss << "(";
  div->lhs.accept(this);
  // oss << ")";
  oss << " / ";
  // oss << "(";
  div->rhs.accept(this);
  oss << ")";
}

void CodeGenC::visitMod(ModHandle mod) {
  oss << "(";
  mod->lhs.accept(this);
  // oss << ")";
  oss << " % ";
  // oss << "(";
  mod->rhs.accept(this);
  oss << ")";
}

void CodeGenC::visitVar(VarHandle var) { oss << var->id; }

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

void CodeGenC::visitTensor(TensorHandle tensor) { oss << tensor->id; }

void CodeGenC::visitVal(ValHandle val) { oss << val->id; }

void CodeGenC::visitDecl(DeclHandle decl) {
  oss << "float ";
  decl->decl.accept(this);
  oss << ";\n";
}

class OutterLoopCollectionHelper : public IRRecursiveVisitor {
 public:
  IRHandle loop;
  std::vector<std::string> outter_loops;
  bool stop;
  OutterLoopCollectionHelper(IRHandle program, IRHandle loop) : loop(loop) {
    stop = false;
    program.accept(this);
  }

  void enter(IRHandle node) override {
    if (stop) return;
    if (node.equals(loop)) {
      stop = true;
      return;
    }
    if (node.Type() == IRNodeType::FOR) {
      outter_loops.push_back(
          node.as<ForNode>()->looping_var_.as<VarNode>()->id);
    }
  }
  void exit(IRHandle node) override {
    if (stop) return;
    if (node.Type() == IRNodeType::FOR) outter_loops.pop_back();
  }
};

void CodeGenC::visitFor(ForHandle loop) {
  VarHandle loop_var = loop->looping_var_.as<VarNode>();

  bool do_parallelization = loop->annotation.parallelization && !parallelized;

  if (do_parallelization) {
    oss << getIndent();
    oss << "{\n";
    indent += 1;
    oss << getIndent();
    oss << "#pragma omp parallel for\n";
    parallelized = true;
  }

  oss << getIndent();
  oss << "for (int ";
  loop->looping_var_.accept(this);
  oss << " = ";
  loop_var->min.accept(this);
  oss << "; ";
  loop->looping_var_.accept(this);
  oss << " < ";
  loop_var->max.accept(this);
  oss << "; ";
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

  if (do_parallelization) {
    indent -= 1;
    oss << getIndent();
    oss << "}\n";
  }
  parallelized = false;
}

void CodeGenC::visitConst(ConstHandle con) { oss << con->name; }

void CodeGenC::visitPrint(PrintHandle print) {
  oss << getIndent();
  oss << "std::cout << ";
  print->print.accept(this);
  oss << " << \"\\n\";\n";
}

void CodeGenC::visitFunc(FuncHandle func) {
  for (int i = 0; i < func->body.size(); i++) {
    func->body[i].accept(this);
  }
}

void CodeGenC::visitMin(MinHandle min) {
  oss << "std::min(";
  min->lhs.accept(this);
  oss << ", ";
  min->rhs.accept(this);
  oss << ")";
}

void CodeGenC::visitMax(MaxHandle max) {
  oss << "std::max(";
  max->lhs.accept(this);
  oss << ", ";
  max->rhs.accept(this);
  oss << ")";
}

void CodeGenC::visitVec(VecHandle vec) { oss << vec->id; }
void CodeGenC::visitVecScalar(VecScalarHandle vecScalar) {
  vec_case(vecScalar->length, "__m128 ", "__m256 ");
  vecScalar->vec.accept(this);
  oss << " = ";
  vec_case(vecScalar->length, "_mm_set1_ps(", "_mm256_set1_ps(");

  vecScalar->scalar.accept(this);
  oss << ")";
  oss << ";\n";
}
void CodeGenC::visitVecLoad(VecLoadHandle vecLoad) {
  vec_case(vecLoad->length, "__m128 ", "__m256 ");

  vecLoad->vec.accept(this);
  oss << " = ";
  vec_case(vecLoad->length, "_mm_loadu_ps(", "_mm256_broadcast_ss(");

  oss << "&";
  vecLoad->data.accept(this);
  oss << ")";
  oss << ";\n";
}
void CodeGenC::visitVecBroadCastLoad(VecBroadCastLoadHandle vecBroadCastLoad) {
  vec_case(vecBroadCastLoad->length, "__m128 ", "__m256 ");

  vecBroadCastLoad->vec.accept(this);
  oss << " = ";
  vec_case(vecBroadCastLoad->length, "_mm_load_ps1(", "_mm256_broadcast_ss(");

  oss << "&";
  vecBroadCastLoad->data.accept(this);
  oss << ")";
  oss << ";\n";
}
void CodeGenC::visitVecStore(VecStoreHandle vecStore) {
  vec_case(vecStore->length, "_mm_storeu_ps(", "_mm256_storeu_ps(");
  oss << "&";
  vecStore->data.accept(this);
  oss << ", ";
  vecStore->vec.accept(this);
  oss << ")";
  oss << ";\n";
}
void CodeGenC::visitVecAdd(VecAddHandle add) {
  vec_case(add->length, "__m128 ", "__m256 ");

  add->vec.accept(this);
  oss << " = ";
  vec_case(add->length, "_mm_add_ps(", "_mm256_add_ps(");

  add->lhs.accept(this);
  oss << ", ";
  add->rhs.accept(this);
  oss << ")";
  oss << ";\n";
}
void CodeGenC::visitVecSub(VecSubHandle sub) {
  vec_case(sub->length, "__m128 ", "__m256 ");

  sub->vec.accept(this);
  oss << " = ";
  vec_case(sub->length, "_mm_sub_ps(", "_mm256_sub_ps(");

  sub->lhs.accept(this);
  oss << ", ";
  sub->rhs.accept(this);
  oss << ")";
  oss << ";\n";
}
void CodeGenC::visitVecMul(VecMulHandle mul) {
  vec_case(mul->length, "__m128 ", "__m256 ");

  mul->vec.accept(this);
  oss << " = ";
  vec_case(mul->length, "_mm_mul_ps(", "_mm256_mul_ps(");

  mul->lhs.accept(this);
  oss << ", ";
  mul->rhs.accept(this);
  oss << ")";
  oss << ";\n";
}
void CodeGenC::visitVecDiv(VecDivHandle div) {
  vec_case(div->length, "__m128 ", "__m256 ");

  div->vec.accept(this);
  oss << " = ";
  vec_case(div->length, "_mm_div_ps(", "_mm256_div_ps(");

  div->lhs.accept(this);
  oss << ", ";
  div->rhs.accept(this);
  oss << ")";
  oss << ";\n";
}

void CodeGenC::vec_case(int vecLen, std::string str1, std::string str2) {
  if (vecLen == 4)
    oss << str1;
  else if (vecLen == 8)
    oss << str2;
  else
    throw std::runtime_error("Unsupported VecLen");
}

}  // namespace polly
