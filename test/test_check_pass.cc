#include "gtest/gtest.h"

#include "dsl/program.h"
#include "dsl/expr.h"
#include "ir/ir_check_pass.h"

using namespace polly;

TEST(IRCheckPass, IRConstantBoundaryCheckVisitor) {
  /// Positive case
  {
    Program prog;
    Tensor A("A", {1024, 1024}), B("B", {1024, 1024}), C("C", {1024, 1024});
    {
      Variable i("i", 0, 1024, 1);
      {
        Variable j("j", 0, 1024, 1);
        {
          Variable k("k", 0, 1024, 1);
          auto x = C(i, i) + i + j;
          C(i, j) = C(i, j) + A(i, k) * B(k, j);
        }
      }
    }
    EXPECT_EQ(prog.IsConstantBoundary(), true);
  }
  /// Negative case
  {
    Program prog;
    Tensor A("A", {1024, 1024}), B("B", {1024, 1024}), C("C", {1024, 1024});
    {
      /// for loop should always start from 0
      Variable i("i", 1, 1024, 1);
      {
        Variable j("j", 0, 1024, 1);
        {
          Variable k("k", 0, 1024, 1);
          auto x = C(i, i) + i + j;
          C(i, j) = C(i, j) + A(i, k) * B(k, j);
        }
      }
    }
    EXPECT_EQ(prog.IsConstantBoundary(), false);
  }
  /// Negative case
  {
    Program prog;
    Tensor A("A", {1024, 1024}), B("B", {1024, 1024}), C("C", {1024, 1024});
    {
      Variable i("i", 0, 1024, 1);
      {
        /// for loop should always increment by 1
        Variable j("j", 0, 1024, 3);
        {
          Variable k("k", 0, 1024, 1);
          auto x = C(i, i) + i + j;
          C(i, j) = C(i, j) + A(i, k) * B(k, j);
        }
      }
    }
    EXPECT_EQ(prog.IsConstantBoundary(), false);
  }
  /// Negative case
  {
    Program prog;
    Tensor A("A", {1024, 1024}), B("B", {1024, 1024}), C("C", {1024, 1024});
    Constant D("D");
    {
      Variable i("i", 0, 1024, 1);
      {
        Variable j("j", 0, 1024, 1);
        {
          /// for loop should always increment by 1
          Variable k("k", 0, D, 1);
          auto x = C(i, i) + i + j;
          C(i, j) = C(i, j) + A(i, k) * B(k, j);
        }
      }
    }
    EXPECT_EQ(prog.IsConstantBoundary(), false);
  }
}

TEST(IRCheckPass, IRDivisibleBoundaryCheckVisitor) {
  {
    Program prog;
    Tensor A("A", {1024, 1024}), B("B", {1024, 1024}), C("C", {1024, 1024});
    {
      Variable i("i", 0, 1024, 1);
      {
        Variable j("j", 0, 1024, 1);
        {
          Variable k("k", 0, 1024, 1);
          C(i, j) = C(i, j) + A(i, k) * B(k, j);
        }
      }
    }
    EXPECT_EQ(prog.IsBoundaryDivisible("i", 4), true);
    EXPECT_EQ(prog.IsBoundaryDivisible("i", 7), false);
    EXPECT_EQ(prog.IsBoundaryDivisible("k", 1024), true);
  }
}