#include "gtest/gtest.h"

#include "lang/program.h"
#include "lang/expr.h"
#include "pass/check/affine_check.h"
#include "pass/check/constant_boundary_check.h"

using namespace polly;

TEST(CODEGEN, CODEGEN_C) {
  {
    Program prog;
    Tensor A({1024}), B({1024});
    {
      Variable i(0, 1024, 1);
      A(i) = i;
      {
        Variable j(0, 1, 1);
        B(i + j) = i + j;
        A(i + 1) = A(i) + B(i + j);
      }
      B(i + 1) = B(i);
    }
    prog.GenerateC();
  }
}

TEST(CODEGEN, CODEGEN_CUDA) {
  {
    Program prog;
    Tensor A({1024}), B({1024});
    {
      Variable i(0, 1024, 1);
      A(i) = i;
      {
        Variable j(0, 1, 1);
        B(i + j) = i + j;
        A(i + 1) = A(i) + B(i + j);
      }
      B(i + 1) = B(i);
    }
    prog.GenerateCuda();
  }
}