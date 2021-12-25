#include "dsl/program.h"
#include "dsl/expr.h"

using namespace polly;

int main() {
  {
    Program prog;
    Tensor A("A", {1024, 1024}), B("B", {1024, 1024}), C("C", {1024, 1024});
    Constant M("M"), N("N"), K("K");
    {
      Variable i("i", 1, 1023, 1);
      {
        Variable j("j", 1, 1023, 1);
        {
          Variable k("k", 1, 1023, 1);
          auto x = C(i, i) + i + j;
          C(i, j) = C(i, j) + A(i, k) * B(k, j);
        }
      }
    }

    prog.GenerateC();
    prog.Reorder("i", "j");
    prog.GenerateC();
    prog.Split("j", 2);
    prog.GenerateC();
    prog.Reorder("j_inner", "k");
    prog.GenerateC();
    prog.Vectorize("j_inner", 2);
    prog.GenerateC();
    prog.Fuse("j_outter", "k");
    prog.GenerateC();

    // prog.Fuse("i", "j");
    // prog.GenerateC();

    std::cout << prog.IsAffineProgram() << std::endl;
  }

  {
    Program prog;
    Tensor A("A", {1024, 1024}), B("B", {3, 3}), C("C", {1022, 1022});
    {
      Variable i("i", 0, 1021, 1);
      {
        Variable j("j", 0, 1021, 1);
        {
          Variable k("k", 0, 2, 1);
          {
            Variable h("h", 0, 2, 1);
            C(i, j) = C(i, j) + A(i + k, j + h) * B(k, h);
          }
        }
      }
    }

    prog.GenerateC();

    std::cout << prog.IsAffineProgram() << std::endl;
  }

  return 0;
}