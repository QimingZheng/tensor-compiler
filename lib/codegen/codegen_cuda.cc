#include "codegen.h"

namespace polly {

std::string CodeGenCuda::genCode(IRHandle program,
                                 std::vector<IRHandle> &tensors) {
  program_ = program;

  oss << R"(
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
)";

  oss << "__global__ void kernel(";

  for (int i = 0; i < tensors.size(); i++) {
    if (i != 0) oss << ", ";
    oss << "float *" << tensors[i].as<TensorNode>()->id;
  }

  oss << ") {\n";

  visit(program);

  oss << "}\n";

  oss << "int main() {\n";

  for (int i = 0; i < tensors.size(); i++) {
    oss << "  float *" << tensors[i].as<TensorNode>()->id << ";\n";
    int64_t sz = 1;
    for (int t = 0; t < tensors[i].as<TensorNode>()->shape.size(); t++) {
      sz *= tensors[i].as<TensorNode>()->shape[t];
    }
    oss << "  cudaMalloc((void **)(&" << tensors[i].as<TensorNode>()->id
        << "), sizeof(float) * " << sz << ");\n";
  }
  oss << "  struct timeval start, end;\n";
  oss << "  gettimeofday(&start, NULL);\n";
  oss << "  kernel<<<64, 1024>>>(";
  for (int i = 0; i < tensors.size(); i++) {
    if (i != 0) oss << ", ";
    oss << tensors[i].as<TensorNode>()->id;
  }
  oss << ");\n";

  for (int i = 0; i < tensors.size(); i++) {
    oss << "  float *" << tensors[i].as<TensorNode>()->id << ";\n";
    int64_t sz = 1;
    for (int t = 0; t < tensors[i].as<TensorNode>()->shape.size(); t++) {
      sz *= tensors[i].as<TensorNode>()->shape[t];
    }
    oss << "  cudaFree(" << tensors[i].as<TensorNode>()->id << ");\n";
  }

  // timing unit: ms
  oss << "  gettimeofday(&end, NULL);\n";
  oss << "  printf(\"%.6f\\n\", (end.tv_sec - start.tv_sec) * 1000L + "
         "(end.tv_usec - start.tv_usec) * 1.0 / 1000L);\n";
  oss << "}\n";
  return oss.str();
}

void CodeGenCuda::visitFor(ForHandle loop) {
  // for (var = min + tid * increment; var < max; var += thread_count *
  // increment)
  VarHandle loop_var = loop->looping_var_.as<VarNode>();

  bool ori = is_outter_most;
  bool do_parallelization = loop->annotation.parallelization && is_outter_most;

  oss << getIndent();
  oss << "for (int ";
  loop->looping_var_.accept(this);
  oss << " = ";
  loop_var->min.accept(this);

  if (do_parallelization) {
    oss << " + (blockIdx.x * blockDim.x + threadIdx.x) * ";
    loop_var->increment.accept(this);
  }

  oss << "; ";
  loop->looping_var_.accept(this);
  oss << " < ";
  loop_var->max.accept(this);
  oss << "; ";
  loop->looping_var_.accept(this);
  oss << " += ";
  loop_var->increment.accept(this);

  if (do_parallelization) {
    oss << " * (blockDim.x * gridDim.x)";
  }

  oss << ") {\n";
  indent += 1;

  is_outter_most = false;
  for (int i = 0; i < loop->body.size(); i++) {
    loop->body[i].accept(this);
  }

  is_outter_most = ori;

  indent -= 1;
  oss << getIndent();
  oss << "}\n";

  if (do_parallelization) {
    oss << getIndent();
    oss << " __syncthreads();\n";
  }
}

void CodeGenCuda::visitFunc(FuncHandle func) {
  for (int i = 0; i < func->body.size(); i++) {
    func->body[i].accept(this);
  }
}

void CodeGenCuda::visitAccess(AccessHandle access) {
  access->tensor.accept(this);
  oss << "[";
  auto tensor = access->tensor.as<TensorNode>();
  std::vector<int64_t> sz(tensor->shape.size());
  int64_t now = 1;
  for (int i = 0; i < sz.size(); i++) {
    sz[sz.size() - i - 1] = now;
    now *= tensor->shape[sz.size() - i - 1];
  }
  for (int i = 0; i < access->indices.size(); i++) {
    if (i != 0) oss << " + ";
    oss << "(";
    access->indices[i].accept(this);
    oss << ") * " << sz[i];
  }
  oss << "]";
}

}  // namespace polly