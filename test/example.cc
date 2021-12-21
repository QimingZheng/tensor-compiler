#include "dsl/program.h"
#include "dsl/expr.h"

using namespace polly;

int main() {
  Program prog;
  Tensor A("A", {1024, 1024}), B("B", {1024, 1024}), C("C", {1024, 1024});
  // Constant M("M"), N("N"), K("K");
  {
    Variable i("i", 1, 10, 1);
    {
      Variable j("j", 1, 10, 1);
      {
        Variable k("k", 1, 10, 1);
        auto x = C(i, i) + i + j;
        C(i, j) = C(i, j) + A(i, k) * B(k, j);
      }
    }
  }

  // prog.GenerateC();
  prog.Reorder("i", "j");
  // prog.GenerateC();
  prog.Split("j", 2);
  // prog.GenerateC();
  prog.Reorder("j_inner", "k");
  // prog.GenerateC();
  prog.Vectorize("j_inner", 2);
  prog.GenerateC();
  prog.Fuse("j_outter", "k");
  // prog.IRGen();
  prog.GenerateC();

  std::cout << prog.IsAffineProgram() << std::endl;

  return 0;
}