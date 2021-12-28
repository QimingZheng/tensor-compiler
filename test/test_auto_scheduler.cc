#include "lang/program.h"
#include "lang/expr.h"

using namespace polly;

int main() {
  Program prog;
  Tensor A("A", {1024, 1024}), B("B", {1024, 1024}), C("C", {1024, 1024});
  Constant M("M"), N("N"), K("K");
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
  srand((unsigned)time(NULL));

  prog.SetSplit("i");
  prog.SetSplit("j");
  prog.SetSplit("k");
  prog.SetReorder("i", "j");
  prog.SetReorder("j", "k");
  prog.AutoTune();

  prog.GenerateC();

  return 0;
}