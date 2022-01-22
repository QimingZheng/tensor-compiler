#include "vectorization.h"
#include "pass/analysis/polyhedral_extraction.h"

namespace polly {

void LoopVectorization::visitInt(IntHandle int_expr) {
  auto vec = VecNode::make("v" + IRNodeKeyGen::GetInstance()->yield(), vecLen);
  vectorizationBody.push_back(
      VecScalarNode::make(vec, IRHandle(int_expr), vecLen));
  node = vec;
}

void LoopVectorization::visitAdd(AddHandle add) {
  auto res = VecNode::make("v" + IRNodeKeyGen::GetInstance()->yield(), vecLen);
  add->lhs.accept(this);
  auto lhs = node;
  add->rhs.accept(this);
  auto rhs = node;
  vectorizationBody.push_back(VecAddNode::make(res, lhs, rhs, vecLen));
  node = res;
}

void LoopVectorization::visitSub(SubHandle sub) {
  auto res = VecNode::make("v" + IRNodeKeyGen::GetInstance()->yield(), vecLen);
  sub->lhs.accept(this);
  auto lhs = node;
  sub->rhs.accept(this);
  auto rhs = node;
  vectorizationBody.push_back(VecSubNode::make(res, lhs, rhs, vecLen));
  node = res;
}

void LoopVectorization::visitMul(MulHandle mul) {
  auto res = VecNode::make("v" + IRNodeKeyGen::GetInstance()->yield(), vecLen);
  mul->lhs.accept(this);
  auto lhs = node;
  mul->rhs.accept(this);
  auto rhs = node;
  vectorizationBody.push_back(VecMulNode::make(res, lhs, rhs, vecLen));
  node = res;
}

void LoopVectorization::visitDiv(DivHandle div) {
  auto res = VecNode::make("v" + IRNodeKeyGen::GetInstance()->yield(), vecLen);
  div->lhs.accept(this);
  auto lhs = node;
  div->rhs.accept(this);
  auto rhs = node;
  vectorizationBody.push_back(VecDivNode::make(res, lhs, rhs, vecLen));
  node = res;
}

void LoopVectorization::visitMod(ModHandle mod) {
  throw std::runtime_error("Cannot vectorized a mod operation");
}

void LoopVectorization::visitVar(VarHandle var) {
  auto vec = VecNode::make("v" + IRNodeKeyGen::GetInstance()->yield(), vecLen);
  vectorizationBody.push_back(VecScalarNode::make(vec, IRHandle(var), vecLen));
  node = vec;
}

void LoopVectorization::visitAccess(AccessHandle access) {
  QuasiAffineExpr expr =
      PolyhedralExtraction::IRHandleToQuasiAffine(access->indices.back());
  auto vec = VecNode::make("v" + IRNodeKeyGen::GetInstance()->yield(), vecLen);
  auto id = loop_.as<ForNode>()->looping_var_.as<VarNode>()->id;
  if (expr.coeffs.find(id) != expr.coeffs.end() && expr.coeffs[id] != 0) {
    vectorizationBody.push_back(
        VecLoadNode::make(vec, IRHandle(access), vecLen));
  } else {
    vectorizationBody.push_back(
        VecBroadCastLoadNode::make(vec, IRHandle(access), vecLen));
  }
  node = vec;
}

void LoopVectorization::visitAssign(AssignmentHandle assign) {
  assign->rhs.accept(this);
  vectorizationBody.push_back(VecStoreNode::make(node, assign->lhs, vecLen));
  node = NullIRHandle;
}

void LoopVectorization::visitTensor(TensorHandle tensor) {
  throw std::runtime_error("Should not visit a Tensor Node");
}

void LoopVectorization::visitFor(ForHandle loop) {
  for (int i = 0; i < loop->body.size(); i++) {
    loop->body[i].accept(this);
    for (auto x : vectorizationBody) {
      assert(x != NullIRHandle);
    }
  }
  loop->body = vectorizationBody;
  vectorizationBody.clear();
  loop->looping_var_.as<VarNode>()->increment = IntNode::make(vecLen);
}

void LoopVectorization::visitConst(ConstHandle con) {
  // TODO
  throw std::runtime_error("Should not visit the Const Node");
}

void LoopVectorization::visitPrint(PrintHandle print) {
  // TODO
  throw std::runtime_error("Cannot vectorize a print statement");
}

void LoopVectorization::visitFunc(FuncHandle func) {
  throw std::runtime_error("Should not visit the For Node");
}

}  // namespace polly