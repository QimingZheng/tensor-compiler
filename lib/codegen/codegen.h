/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:32:55
 * @Last Modified by:   Qiming Zheng
 * @Last Modified time: 2022-01-18 20:32:55
 * @CopyRight: Qiming Zheng
 */
#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

namespace polly {

class CodeGen : public IRVisitor {
 public:
};

const std::string C_Heaader = R"(
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mmintrin.h>   // mmx
#include <xmmintrin.h>  // sse
#include <emmintrin.h>  // sse2
#include <pmmintrin.h>  // sse3
#include <immintrin.h>
)";

class CodeGenC : public IRVisitor {
  std::string getIndent() {
    std::string ret = "";
    for (int i = 0; i < indent; i++) {
      ret += '\t';
    }
    return ret;
  }
  std::ostream &oss;
  int indent = 1;

 public:
  CodeGenC(std::ostream &os) : oss(os) {}
  void genCode(IRHandle program, std::vector<IRHandle> &tensors);
  void visitInt(IntHandle int_expr) override;
  void visitAdd(AddHandle add) override;
  void visitSub(SubHandle sub) override;
  void visitMul(MulHandle mul) override;
  void visitDiv(DivHandle div) override;
  void visitMod(ModHandle mod) override;
  void visitVar(VarHandle var) override;
  void visitAccess(AccessHandle access) override;
  void visitAssign(AssignmentHandle assign) override;
  void visitTensor(TensorHandle tensor) override;
  void visitFor(ForHandle loop) override;
  void visitConst(ConstHandle con) override;
  void visitPrint(PrintHandle print) override;
  void visitFunc(FuncHandle func) override;

  void visitVec(VecHandle vec) override;
  void visitVecScalar(VecScalarHandle vecScalar) override;
  void visitVecLoad(VecLoadHandle vecLoad) override;
  void visitVecBroadCastLoad(VecBroadCastLoadHandle vecBroadCastLoad) override;
  void visitVecStore(VecStoreHandle vecStore) override;
  void visitVecAdd(VecAddHandle add) override;
  void visitVecSub(VecSubHandle sub) override;
  void visitVecMul(VecMulHandle mul) override;
  void visitVecDiv(VecDivHandle div) override;

  void vec_case(int vecLen, std::string str1, std::string str2);
};

class CodeGenCuda : public CodeGen {
 public:
};

}  // namespace polly