#include "dsl/program.h"
#include "dsl/expr.h"

using namespace polly;

int main() {
  Program prog;
  Tensor A("A"), B("B"), C("C");
  {
    Variable i("i", 1, 1, 1);
    {
      Variable j("j", 1, 1, 1);
      {
        Variable k("k", 1, 1, 1);
        auto x = C(i, i) + i + j;
        C(i, j) = C(i, j) + A(i, k) * B(k, j);
      }
    }
  }

  prog.GenerateC();
  prog.IRGen();
  prog.Reorder("i", "j");
  prog.IRGen();
  prog.Split("j", 2);
  prog.IRGen();
  prog.Reorder("j_inner", "k");
  prog.IRGen();
  prog.Vectorize("j_inner", 2);
  prog.IRGen();

  std::cout << prog.IsAffineProgram() << std::endl;

  return 0;
}