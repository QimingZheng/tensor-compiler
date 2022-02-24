/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:32:55
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-26 21:56:31
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
#include <omp.h>
#include <sys/time.h>
)";

/*!
 * \brief The code generator for C code.
 */
class CodeGenC : public IRVisitor {
 public:
  static std::string C_Runtime_Deps;

  /// Used by the parallelization.
  std::vector<std::string> tensor_name;
  std::vector<std::vector<int64_t>> tensor_shape;

  int parallel_loop_count;
  int worker_size = 16;
  bool parallelized = false;

  struct method_arguments {
    method_arguments(std::string method_name,
                     std::vector<std::string> loop_var_names, int level,
                     int worker_size, IRHandle parallel_loop)
        : method_name(method_name),
          loop_var_names(loop_var_names),
          level(level),
          worker_size(worker_size),
          parallel_loop(parallel_loop) {}
    std::string method_name;
    std::vector<std::string> loop_var_names;
    int level;
    int worker_size;
    IRHandle parallel_loop;
  };

  std::vector<method_arguments> methods_;
  std::vector<std::string> method_decls_;

  IRHandle program_;

 public:
  CodeGenC() {
    parallel_loop_count = 0;
    parallelized = false;
  }
  std::string genCode(IRHandle program, std::vector<IRHandle> &tensors,
                      std::string program_name);

  std::string genTensors(std::vector<IRHandle> &tensors);
  std::string genTensorParam(std::vector<IRHandle> &tensors);

  void visitInt(IntHandle int_expr) override;
  void visitFloat(FloatHandle float_expr) override;
  void visitAdd(AddHandle add) override;
  void visitSub(SubHandle sub) override;
  void visitMul(MulHandle mul) override;
  void visitDiv(DivHandle div) override;
  void visitMod(ModHandle mod) override;
  void visitVar(VarHandle var) override;
  void visitAccess(AccessHandle access) override;
  void visitAssign(AssignmentHandle assign) override;
  void visitTensor(TensorHandle tensor) override;
  void visitVal(ValHandle val) override;
  void visitDecl(DeclHandle decl) override;
  void visitFor(ForHandle loop) override;
  void visitConst(ConstHandle con) override;
  void visitPrint(PrintHandle print) override;
  void visitFunc(FuncHandle func) override;

  void visitMin(MinHandle min) override;
  void visitMax(MaxHandle max) override;

  void visitVec(VecHandle vec) override;
  void visitVecScalar(VecScalarHandle vecScalar) override;
  void visitVecLoad(VecLoadHandle vecLoad) override;
  void visitVecBroadCastLoad(VecBroadCastLoadHandle vecBroadCastLoad) override;
  void visitVecStore(VecStoreHandle vecStore) override;
  void visitVecAdd(VecAddHandle add) override;
  void visitVecSub(VecSubHandle sub) override;
  void visitVecMul(VecMulHandle mul) override;
  void visitVecDiv(VecDivHandle div) override;

  void create_method(std::string method_name,
                     std::vector<std::string> tensor_name,
                     std::vector<std::string> outter_loop_vars, int level,
                     IRHandle loop);
  std::string tensor_shape_str(std::vector<int64_t> shape);
  void vec_case(int vecLen, std::string str1, std::string str2);
  std::string getIndent() {
    std::string ret = "";
    for (int i = 0; i < indent; i++) {
      ret += '\t';
    }
    return ret;
  }
  std::ostringstream oss;
  int indent = 1;
};

/*!
 * \brief The code generator for CUDA (NVIDIA) code.
 */
class CodeGenCuda : public CodeGenC {
 public:
  CodeGenCuda() { is_outter_most = false; }
  std::string genCode(IRHandle program, std::vector<IRHandle> &tensors);
  void visitFor(ForHandle loop) override;
  void visitFunc(FuncHandle func) override;
  void visitAccess(AccessHandle access) override;

  bool is_outter_most = false;
};

}  // namespace polly