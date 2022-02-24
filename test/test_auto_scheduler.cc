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
  // prog.AutoTune("RandomSearch");

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
        // C(i, j) = C(i, j) + A(i, k) * B(k, j);
        C(i, j) += A(i, k) * B(k, j);
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

void Cholesky() {
  Program prog;
  Tensor A({1024, 1024}), L({1024, 1024}), U({1024, 1024});
  {
    Variable j(0, 1024, 1);
    {
      Variable k(0, j, 1);
      { Variable i(0, k, 1); }
    }
  }
}

void CONV() {
  Program prog;
  int BATCH = 128;
  int CHANNEL = 3;
  int H = 1024, W = 1024;
  int KH = 5, KW = 5;
  Tensor I({128, 3, 1024, 1024}), O({128, 3, 1024, 1024}), K({5, 5});
  {
    Variable i(0, BATCH, 1);
    {
      Variable c(0, CHANNEL, 1);
      {
        Variable h(0, H - KH, 1);
        {
          Variable w(0, W - KW, 1);
          {
            Variable Kh(0, KH, 1);
            {
              Variable Kw(0, KW, 1);
              O(i, c, h, w) =
                  O(i, c, h, w) + K(Kh, Kw) * I(i, c, h + Kh, w + Kw);
            }
          }
        }
      }
    }
  }
  srand((unsigned)time(NULL));

  prog.AutoTune("BeamSearch");
  // prog.AutoTune("RandomSearch");

  prog.GenerateC();
}

void BLUR() {
  Program prog;
  int H = 1024, W = 1024;
  Tensor I({1024, 1024}), O({1024, 1024}), B({1024, 1024});
  {
    Variable h(1, H - 1, 1);
    {
      Variable w(0, W, 1);
      {
        Variable i(-1, 2, 1);
        B(h, w) = B(h, w) + I(h - i, w);
      }
    }
  }
  {
    Variable h(0, H, 1);
    {
      Variable w(1, W - 1, 1);
      {
        Variable i(-1, 2, 1);
        O(h, w) = O(h, w) + B(h, w + i);
      }
    }
  }
  srand((unsigned)time(NULL));

  prog.AutoTune("BeamSearch");
  // prog.AutoTune("RandomSearch");

  prog.GenerateC();
}

void BlackSholes() {
  // TODO
}

int main() {
  // GEMM();
  LU();
  // CONV();
  // BLUR();
  return 0;
}
