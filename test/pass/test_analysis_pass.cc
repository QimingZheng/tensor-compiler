#include "gtest/gtest.h"

#include "lang/program.h"
#include "lang/expr.h"

#include "pass/analysis/analysis_pass.h"
#include "pass/analysis/polyhedral_extraction.h"
#include "pass/analysis/data_dependency_model.h"

using namespace polly;

TEST(POLYHEDRAL_ANALYSIS_PASS, MODEL_EXTRACTION) {
  {
    Program prog;
    Tensor A({1024, 1024}), B({1024, 1024});
    {
      {
        Variable i(0, 1024, 1);
        A(i) = i * 2;
      }
      {
        Variable i(0, 1024, 1);
        B(i) = A(i) + i * 3;
      }
    }
    PolyhedralExtraction extraction(prog.module_.GetRoot());
    PolyhedralModel model = extraction.model;
    // for (auto st : model.statements_) {
    //   std::cout << st.DbgMsg();
    // }
  }
}

TEST(POLYHEDRAL_ANALYSIS_PASS, DATA_DEPENDENCY_MODEL) {
  {
    Program prog;
    Tensor A({1024, 1024}), B({1024, 1024});
    {
      {
        Variable i(0, 1024, 1);
        A(i) = i * 2;
      }
      {
        Variable i(0, 1024, 1);
        B(i) = A(i) + i * 3;
      }
    }
    PolyhedralExtraction extraction(prog.module_.GetRoot());
    PolyhedralModel model = extraction.model;
    // for (auto st : model.statements_) {
    //   std::cout << st.DbgMsg();
    // }
    solver::context ctx;
    DataDependencyModel dep(ctx, model);
  }
}
