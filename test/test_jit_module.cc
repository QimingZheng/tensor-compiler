#include "lang/program.h"
#include "lang/expr.h"
#include "jit/jit_module.h"

using namespace polly;

int main() {
  Program prog;
  Tensor A({4, 4}), B({4, 4}), C({4, 4});
  {
    Variable i(0, 4, 1);
    {
      Variable j(0, 4, 1);
      {
        Variable k(0, 4, 1);
        auto x = C(i, i) + i + j;
        C(i, j) = C(i, j) + A(i, k) * B(k, j);
      }
      Print(C(i, j));
    }
  }
  prog.GenerateC();
  prog.RunJit();
  return 0;
}