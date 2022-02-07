#include "lang/program.h"
#include "lang/expr.h"
#include "pass/transform/fission.h"
#include "pass/transform/fussion.h"
using namespace polly;

void LU() {
  Program prog;
  Tensor A({1024, 1024}), L({1024, 1024}), U({1024, 1024});

  {
    Variable i(0, 1024, 1);
    U(i, i) = 1;
  }
  {
    Variable i(0, 1024, 1);
    {
      Variable j(i, 1024, 1);
      L(j, i) = A(j, i);
      {
        Variable k(0, i, 1);
        L(j, i) = L(j, i) - L(j, k) * U(k, i);
      }
    }
    {
      Variable j(i + 1, 1024, 1);
      U(i, j) = A(i, j) / L(i, i);
      {
        Variable k(0, i, 1);
        U(i, j) = U(i, j) - ((L(i, k) * U(k, j))) / L(i, i);
      }
    }
  }
  srand((unsigned)time(NULL));

  prog.AutoTune("BeamSearch");

  prog.GenerateC();
}

void GEMM() {
  Program prog;
  Tensor A({1024, 1024}), B({1024, 1024}), C({1024, 1024});
  // Constant M("M"), N("N"), K("K");
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
        auto x = C(i, i) + i + j;
        C(i, j) = C(i, j) + A(i, k) * B(k, j);
      }
    }
  }
  srand((unsigned)time(NULL));

  prog.SetSplit(I);
  prog.SetSplit(J);
  prog.SetSplit(K);
  prog.SetReorder(I, J);
  prog.SetReorder(J, K);
  // prog.AutoTune("RandomSearch");
  prog.AutoTune("BeamSearch");

  prog.GenerateC();
}

int main() {
  // GEMM();
  LU();
  return 0;
}
