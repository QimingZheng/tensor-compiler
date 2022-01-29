#include "lang/program.h"
#include "lang/expr.h"
#include "pass/transform/fission.h"
#include "pass/transform/fussion.h"
using namespace polly;

int main() {
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
  prog.AutoTune("RandomSearch");

  prog.GenerateC();

  return 0;
}