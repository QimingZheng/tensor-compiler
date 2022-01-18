#include "gtest/gtest.h"

#include "lang/program.h"
#include "lang/expr.h"

using namespace polly;

TEST(IRCheckPass, IRConstantBoundaryCheckVisitor) {
  /// Positive case
  {
    Program prog;
    Tensor A({1024, 1024}), B({1024, 1024}), C({1024, 1024});
    {
      Variable i(0, 1024, 1);
      {
        Variable j(0, 1024, 1);
        {
          Variable k(0, 1024, 1);
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
    Tensor A({1024, 1024}), B({1024, 1024}), C({1024, 1024});
    {
      /// for loop should always start from 0
      Variable i(1, 1024, 1);
      {
        Variable j(0, 1024, 1);
        {
          Variable k(0, 1024, 1);
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
    Tensor A({1024, 1024}), B({1024, 1024}), C({1024, 1024});
    {
      Variable i(0, 1024, 1);
      {
        /// for loop should always increment by 1
        Variable j(0, 1024, 3);
        {
          Variable k(0, 1024, 1);
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
    Tensor A({1024, 1024}), B({1024, 1024}), C({1024, 1024});
    {
      Variable i(0, 1024, 1);
      {
        Variable j(0, 1024, 1);
        {
          /// for loop should always increment by 1
          Variable k(0, j, 1);
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
    Tensor A({1024, 1024}), B({1024, 1024}), C({1024, 1024});
    IRNodeKey I, J, K;
    {
      Variable i(0, 1024, 1);
      I = i.id;
      {
        Variable j(0, 1024, 1);
        J = j.id;
        {
          Variable k(0, 1024, 1);
          K = k.id;
          C(i, j) = C(i, j) + A(i, k) * B(k, j);
        }
      }
    }
    EXPECT_EQ(prog.IsBoundaryDivisible(I, 4), true);
    EXPECT_EQ(prog.IsBoundaryDivisible(I, 7), false);
    EXPECT_EQ(prog.IsBoundaryDivisible(K, 1024), true);
  }
}