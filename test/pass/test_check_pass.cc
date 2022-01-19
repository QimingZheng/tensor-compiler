#include "gtest/gtest.h"

#include "lang/program.h"
#include "lang/expr.h"
#include "pass/check/affine_check.h"
#include "pass/check/constant_boundary_check.h"

using namespace polly;

void affineCheck(IRHandle root, bool expected) {
  auto ret = AffineCheck::runPass(
      std::shared_ptr<AffineCheck::Arg>(new AffineCheck::Arg(root)));
  EXPECT_EQ(PassRet::as<AffineCheck::Ret>(ret)->isAffine, expected);
}

TEST(AffineCheck, AffineCheck) {
  {
    Program prog;
    Tensor A({1024, 1024}), B({1024, 1024}), C({1024, 1024});
    {
      Variable i(0, 1024, 1);
      {
        Variable j(0, 1024, 1);
        {
          Variable k(0, 1024, 1);
          C(i, j) = C(i, j) + A(i, k) * B(k, j);
        }
      }
    }
    affineCheck(prog.module_.GetRoot(), true);
  }
  {
    Program prog;
    Tensor A({1024});
    {
      Variable i(0, 1024, 1);
      A(i) = A(i + 10) / 2;
    }
    affineCheck(prog.module_.GetRoot(), true);
  }
  {
    Program prog;
    Tensor A({1024});
    {
      Variable i(0, 1024, 1);
      A(i) = A(i * i /* NonLinear Indexing */ + 10) / 2;
    }
    affineCheck(prog.module_.GetRoot(), false);
  }
  {
    Program prog;
    Tensor A({1024});
    {
      Variable i(0, 1024, 1);
      A(i) = A(i + 10) * A(i) + i * i;
    }
    affineCheck(prog.module_.GetRoot(), true);
  }
  {
    Program prog;
    Tensor A({1024});
    {
      Variable i(0, 1024, 1);
      A(i) = A(i + 10) * A(i) + i * i;
      {
        Variable j(0, 1024, 1);
        A(i + j) = A(i - A(j) /* NonLinear Indexing */);
      }
    }
    affineCheck(prog.module_.GetRoot(), false);
  }
}

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
    EXPECT_EQ(prog.IsBoundaryDivisible(K, 0), false);
    EXPECT_EQ(prog.IsBoundaryDivisible(K, -10), false);
  }
}