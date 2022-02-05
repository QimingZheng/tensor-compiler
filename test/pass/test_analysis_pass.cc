#include "gtest/gtest.h"

#include "lang/program.h"
#include "lang/expr.h"

#include "pass/analysis/analysis_pass.h"
#include "pass/analysis/polyhedral_extraction.h"
#include "pass/analysis/data_dependency_model.h"
#include "pass/analysis/transform_analysis_pass.h"
#include "pass/analysis/parallelization_analysis_pass.h"

#include "pass/transform/fussion.h"
#include "pass/transform/fission.h"
#include "pass/transform/reorder.h"

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

TEST(TRANSFORM_ANALYSIS, FUSSION_TRANSFORM_ANALYSIS) {
  // Positive Case
  {
    Program prog;
    Tensor A({1024}), B({1024});
    IRNodeKey I1, I2;
    {
      {
        Variable i(0, 1024, 1);
        I1 = i.id;
        A(i) = i * 2;
      }
      {
        Variable i(0, 1024, 1);
        I2 = i.id;
        B(i) = A(i) + i * 3;
      }
    }

    auto fussion_module = prog.module_.CreateSubSpace();
    auto root = fussion_module.GetRoot();
    auto loop_1 = fussion_module.GetLoop(I1);
    auto loop_2 = fussion_module.GetLoop(I2);

    FussionTransform::runPass(
        FussionTransform::Arg::create(root, loop_1, loop_2));

    auto ret = FussionTransformAnalysisPass::runPass(
        FussionTransformAnalysisPass::Arg::create(prog.module_.GetRoot(),
                                                  root));

    EXPECT_EQ(PassRet::as<FussionTransformAnalysisPass::Ret>(ret)->legal, true);
  }
  // Positive Case
  {
    Program prog;
    Tensor A({1024});
    IRNodeKey I1, I2;
    {
      {
        Variable i(1, 1024, 1);
        I1 = i.id;
        A(i) = i * 2;
      }
      {
        Variable i(1, 1024, 1);
        I2 = i.id;
        A(i - 1) = A(i) + i * 3;
      }
    }

    auto fussion_module = prog.module_.CreateSubSpace();
    auto root = fussion_module.GetRoot();
    auto loop_1 = fussion_module.GetLoop(I1);
    auto loop_2 = fussion_module.GetLoop(I2);

    FussionTransform::runPass(
        FussionTransform::Arg::create(root, loop_1, loop_2));

    auto ret = FussionTransformAnalysisPass::runPass(
        FussionTransformAnalysisPass::Arg::create(prog.module_.GetRoot(),
                                                  root));

    EXPECT_EQ(PassRet::as<FussionTransformAnalysisPass::Ret>(ret)->legal, true);
  }
  // Negative Case
  {
    Program prog;
    Tensor A({1024});
    IRNodeKey I1, I2;
    {
      {
        Variable i(1, 1024, 1);
        I1 = i.id;
        A(i) = i * 2;
      }
      {
        Variable i(1, 1024, 1);
        I2 = i.id;
        A(i + 1) = A(i) + i * 3;
      }
    }

    auto fussion_module = prog.module_.CreateSubSpace();
    auto root = fussion_module.GetRoot();
    auto loop_1 = fussion_module.GetLoop(I1);
    auto loop_2 = fussion_module.GetLoop(I2);

    FussionTransform::runPass(
        FussionTransform::Arg::create(root, loop_1, loop_2));

    auto ret = FussionTransformAnalysisPass::runPass(
        FussionTransformAnalysisPass::Arg::create(prog.module_.GetRoot(),
                                                  root));

    EXPECT_EQ(PassRet::as<FussionTransformAnalysisPass::Ret>(ret)->legal,
              false);
  }
}

TEST(TRANSFORM_ANALYSIS, FISSION_TRANSFORM_ANALYSIS) {
  // Positive Case
  {
    Program prog;
    Tensor A({1024});
    IRNodeKey I;
    {
      {
        Variable i(0, 1024, 1);
        I = i.id;
        A(i) = i * 2;
        A(i) = A(i) + i * 3;
      }
    }

    auto fission_module = prog.module_.CreateSubSpace();
    auto root = fission_module.GetRoot();
    auto loop = fission_module.GetLoop(I);

    FissionTransform::runPass(FissionTransform::Arg::create(root, loop));

    auto ret = FissionTransformAnalysisPass::runPass(
        FissionTransformAnalysisPass::Arg::create(prog.module_.GetRoot(),
                                                  root));

    EXPECT_EQ(PassRet::as<FissionTransformAnalysisPass::Ret>(ret)->legal, true);
  }
  // Negative Case
  {
    Program prog;
    Tensor A({1024});
    IRNodeKey I;
    {
      {
        Variable i(0, 1023, 1);
        I = i.id;
        A(i) = i * 2;
        A(i + 1) = A(i) + i * 3;
      }
    }

    auto fission_module = prog.module_.CreateSubSpace();
    auto root = fission_module.GetRoot();
    auto loop = fission_module.GetLoop(I);

    FissionTransform::runPass(FissionTransform::Arg::create(root, loop));

    auto ret = FissionTransformAnalysisPass::runPass(
        FissionTransformAnalysisPass::Arg::create(prog.module_.GetRoot(),
                                                  root));

    EXPECT_EQ(PassRet::as<FissionTransformAnalysisPass::Ret>(ret)->legal,
              false);
  }
}

TEST(TRANSFORM_ANALYSIS, REORDER_TRANSFORM_ANALYSIS) {
  // Positive Case
  {
    Program prog;
    Tensor A({1024, 1024}), B({1024, 1024});
    IRNodeKey I, J;
    {
      Variable i(0, 1024, 1);
      I = i.id;
      {
        Variable j(0, 1024, 1);
        J = j.id;
        A(i, j) = A(i, j) * (B(i, j) + B(j, i));
      }
    }

    auto reorder_module = prog.module_.CreateSubSpace();
    auto root = reorder_module.GetRoot();
    auto i_loop = reorder_module.GetLoop(I);
    auto j_loop = reorder_module.GetLoop(J);

    LoopReorder::runPass(
        LoopReorder::Arg::create(root, i_loop.as<ForNode>()->looping_var_,
                                 j_loop.as<ForNode>()->looping_var_));

    auto ret = ReorderTransformAnalysisPass::runPass(
        ReorderTransformAnalysisPass::Arg::create(prog.module_.GetRoot(),
                                                  root));

    EXPECT_EQ(PassRet::as<ReorderTransformAnalysisPass::Ret>(ret)->legal, true);
  }
  // Negative Case
  {
    Program prog;
    Tensor A({1024, 1024}), B({1024, 1024});
    IRNodeKey I, J;
    {
      Variable i(0, 1024, 1);
      I = i.id;
      {
        Variable j(0, 1024, 1);
        J = j.id;
        A(i, j) = A(j, i) * (B(i, j) + B(j, i));
      }
    }

    auto reorder_module = prog.module_.CreateSubSpace();
    auto root = reorder_module.GetRoot();
    auto i_loop = reorder_module.GetLoop(I);
    auto j_loop = reorder_module.GetLoop(J);

    LoopReorder::runPass(
        LoopReorder::Arg::create(root, i_loop.as<ForNode>()->looping_var_,
                                 j_loop.as<ForNode>()->looping_var_));

    auto ret = ReorderTransformAnalysisPass::runPass(
        ReorderTransformAnalysisPass::Arg::create(prog.module_.GetRoot(),
                                                  root));

    EXPECT_EQ(PassRet::as<ReorderTransformAnalysisPass::Ret>(ret)->legal,
              false);
  }
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

    auto reorder_module = prog.module_.CreateSubSpace();
    auto root = reorder_module.GetRoot();
    auto j_loop = reorder_module.GetLoop(J);
    auto k_loop = reorder_module.GetLoop(K);

    LoopReorder::runPass(
        LoopReorder::Arg::create(root, j_loop.as<ForNode>()->looping_var_,
                                 k_loop.as<ForNode>()->looping_var_));

    auto ret = ReorderTransformAnalysisPass::runPass(
        ReorderTransformAnalysisPass::Arg::create(prog.module_.GetRoot(),
                                                  root));

    EXPECT_EQ(PassRet::as<ReorderTransformAnalysisPass::Ret>(ret)->legal, true);
  }
}

TEST(PARALLEL_ANALYSIS, PARALLEL_ANALYSIS) {
  // Positive Case
  {
    Program prog;
    Tensor A({1024});
    IRNodeKey I, J;
    {
      Variable i(0, 1024, 1);
      I = i.id;
      A(i) = A(i) * i + i;
    }

    auto par_module = prog.module_.CreateSubSpace();
    auto root = par_module.GetRoot();
    auto i_loop = par_module.GetLoop(I);

    auto arg = ParallelizationAnalysisPass::Arg::create(root, i_loop);

    auto ret = ParallelizationAnalysisPass::runPass(arg);

    EXPECT_EQ(PassRet::as<ParallelizationAnalysisPass::Ret>(ret)->legal, true);
  }
  // Positive & Negative Case
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

    auto par_module = prog.module_.CreateSubSpace();
    auto root = par_module.GetRoot();
    auto i_loop = par_module.GetLoop(I);
    auto j_loop = par_module.GetLoop(J);
    auto k_loop = par_module.GetLoop(K);

    EXPECT_EQ(PassRet::as<ParallelizationAnalysisPass::Ret>(
                  ParallelizationAnalysisPass::runPass(
                      ParallelizationAnalysisPass::Arg::create(root, i_loop)))
                  ->legal,
              true);

    EXPECT_EQ(PassRet::as<ParallelizationAnalysisPass::Ret>(
                  ParallelizationAnalysisPass::runPass(
                      ParallelizationAnalysisPass::Arg::create(root, j_loop)))
                  ->legal,
              true);
    EXPECT_EQ(PassRet::as<ParallelizationAnalysisPass::Ret>(
                  ParallelizationAnalysisPass::runPass(
                      ParallelizationAnalysisPass::Arg::create(root, k_loop)))
                  ->legal,
              false);
  }
  // Negative Case
  {
    Program prog;
    Tensor A({1024});
    IRNodeKey I, J;
    {
      Variable i(0, 1023, 1);
      I = i.id;
      A(i) = A(i + 1) * i + i;
    }

    auto par_module = prog.module_.CreateSubSpace();
    auto root = par_module.GetRoot();
    auto i_loop = par_module.GetLoop(I);

    auto arg = ParallelizationAnalysisPass::Arg::create(root, i_loop);

    auto ret = ParallelizationAnalysisPass::runPass(arg);

    EXPECT_EQ(PassRet::as<ParallelizationAnalysisPass::Ret>(ret)->legal, false);
  }
}