#include "lang/program.h"
#include "lang/expr.h"

#include "pass/analysis/analysis_pass.h"
#include "pass/analysis/polyhedral_extraction.h"
#include "pass/transform/fission.h"
#include "pass/transform/fussion.h"
#include "pass/transform/normalization.h"
#include "pass/transform/reorder.h"
#include "pass/transform/split.h"
#include "pass/transform/vectorization.h"
#include "pass/analysis/parallelization_analysis_pass.h"

#include "pass/parallelization/parallel_utils.h"
#include "pass/parallelization/sync_parallel.h"

using namespace polly;

int main() {
  {
    Program prog;
    Tensor A({1024, 1024}), B({1024, 1024}), C({1024, 1024});
    IRNodeKey I, J, K;
    {
      Variable i(1, 1023, 1);
      I = i.id;
      {
        Variable j(1, 1023, 1);
        J = j.id;
        {
          Variable k(1, 1024, 1);
          K = k.id;
          auto x = C(i, i) + i + j;
          C(i, j) = C(i, j) + A(i, k) * B(k, j);
        }
      }
    }

    prog.GenerateC();
    prog.Reorder(I, J);
    prog.GenerateC();
    prog.Split(J, 2);
    prog.GenerateC();
    prog.Reorder(J + "_inner", K);
    prog.GenerateC();
    prog.Vectorize(J + "_inner", 2);
    prog.GenerateC();
    prog.Fuse(J + "_outter", K);
    prog.GenerateC();

    // prog.Fuse("i", "j");
    // prog.GenerateC();

    std::cout << prog.IsAffineProgram() << std::endl;
  }

  {
    Program prog;
    Tensor A({1024, 1024}), B({3, 3}), C({1022, 1022});
    {
      Variable i(0, 1021, 1);
      {
        Variable j(0, 1021, 1);
        {
          Variable k(0, 2, 1);
          {
            Variable h(0, 2, 1);
            C(i, j) = C(i, j) + A(i + k, j + h) * B(k, h);
          }
        }
      }
    }

    prog.Unroll();
    prog.GenerateC();

    std::cout << prog.IsAffineProgram() << std::endl;
  }

  {
    Program prog;
    Tensor A({1024, 1024}), B({1024, 1024});
    IRNodeKey I, J;
    {
      {
        Variable i(0, 1024, 1);
        I = i.id;
        A(i) = i * 2;
      }
      {
        Variable i(0, 1024, 1);
        J = i.id;
        B(i) = A(i) + i * 3;
      }
    }

    solver::context ctx;

    IRHandle ori = prog.module_.CreateSubSpace().GetRoot();

    PolyhedralModel oriModel = PolyhedralExtraction(ori).model;
    std::cout << "==============\n";
    for (auto st : oriModel.statements_) {
      std::cout << st.DbgMsg();
    }
    std::cout << "==============\n";

    DataDependencyModel oriDep(ctx, PolyhedralExtraction(ori).model);

    IRHandle first_loop = prog.module_.GetLoop(I);
    IRHandle second_loop = prog.module_.GetLoop(J);
    FussionTransform::runPass(
        std::shared_ptr<FussionTransform::Arg>(new FussionTransform::Arg(
            prog.module_.GetRoot(), first_loop, second_loop)));

    // prog.GenerateC();
    IRHandle transformed = prog.module_.GetRoot();

    PolyhedralModel transformedModel = PolyhedralExtraction(transformed).model;
    std::cout << "==============\n";
    for (auto st : transformedModel.statements_) {
      std::cout << st.DbgMsg();
    }
    std::cout << "==============\n";
    DataDependencyModel dep(ctx, transformedModel);

    auto ret = PolyhedralAnalysisPass::runPass(
        std::shared_ptr<PolyhedralAnalysisPass::Arg>(
            new PolyhedralAnalysisPass::Arg(
                ctx, ori, transformed,
                DataDependencyModel::CreateTransformMap(ctx, oriModel,
                                                        transformedModel))));
    std::cout << PassRet::as<PolyhedralAnalysisPass::Ret>(ret)->hasConflicts
              << std::endl;
  }

  {
    Program prog;
    Tensor A({1024, 1024}), B({1024, 1024});
    IRNodeKey I;
    {
      {
        Variable i(0, 1024, 1);
        I = i.id;
        A(i) = i * 2;
        B(i) = A(i) + i * 3;
      }
    }

    solver::context ctx;

    IRHandle ori = prog.module_.CreateSubSpace().GetRoot();

    PolyhedralModel oriModel = PolyhedralExtraction(ori).model;
    std::cout << "==============\n";
    for (auto st : oriModel.statements_) {
      std::cout << st.DbgMsg();
    }
    std::cout << "==============\n";

    DataDependencyModel oriDep(ctx, PolyhedralExtraction(ori).model);

    std::cout << "==============\n";
    IRHandle loop = prog.module_.GetLoop(I);

    std::cout << "==============\n";
    FissionTransform::runPass(std::shared_ptr<FissionTransform::Arg>(
        new FissionTransform::Arg(prog.module_.GetRoot(), loop)));

    std::cout << "==============\n";
    IRHandle transformed = prog.module_.GetRoot();

    PolyhedralModel transformedModel = PolyhedralExtraction(transformed).model;
    std::cout << "==============\n";
    for (auto st : transformedModel.statements_) {
      std::cout << st.DbgMsg();
    }
    std::cout << "==============\n";
    DataDependencyModel dep(ctx, transformedModel);

    auto ret = PolyhedralAnalysisPass::runPass(
        std::shared_ptr<PolyhedralAnalysisPass::Arg>(
            new PolyhedralAnalysisPass::Arg(
                ctx, ori, transformed,
                DataDependencyModel::CreateTransformMap(ctx, oriModel,
                                                        transformedModel))));
    std::cout << PassRet::as<PolyhedralAnalysisPass::Ret>(ret)->hasConflicts
              << std::endl;
  }

  {
    Program prog;
    Tensor A({1024});
    {
      Variable i(1, 1024, 1);
      A(i) = i + 2 - 0 * A(i - 0);
    }
    auto nodes = prog.module_.GetIRNodes();
    std::cout << (nodes.find(IntNode::make(0)) == nodes.end()) << "\n";
    std::cout << (nodes.find(IntNode::make(2)) == nodes.end()) << "\n";
    ConstantFoldingPass::runPass(std::shared_ptr<ConstantFoldingPass::Arg>(
        new ConstantFoldingPass::Arg(prog.module_.GetRoot())));
    prog.GenerateC();
  }
  {
    Program prog;
    Tensor A({1024, 1024}), B({1024, 1024}), C({1024, 1024});
    IRNodeKey I, J, K;
    {
      Variable i(0, 1024, 1);
      I = i.id;
      {
        Variable j(0, i + 1, 1);
        J = j.id;
        {
          Variable k(0, 1024, 1);
          K = k.id;
          C(i, j) = C(i, j) + A(i, k) * B(k, j);
        }
      }
    }
    auto root = prog.module_.GetRoot();
    auto i_loop = prog.module_.GetLoop(I).as<ForNode>()->looping_var_;
    auto j_loop = prog.module_.GetLoop(J).as<ForNode>()->looping_var_;
    LoopReorder::runPass(LoopReorder::Arg::create(root, i_loop, j_loop));
    // ConstantFoldingPass::runPass(
    //     ConstantFoldingPass::Arg::create(prog.module_.GetRoot()));
    prog.GenerateC();
  }
  {
    Program prog;
    Tensor A({1024});
    IRNodeKey I;
    {
      Variable i(0, 16, 1);
      I = i.id;
      A(i) = i + A(i);
    }
    auto i_loop = prog.module_.GetLoop(I);
    LoopSplit::runPass(
        LoopSplit::Arg::create(prog.module_.GetRoot(), i_loop, 4));
    prog.GenerateC();
  }
  {
    Program prog;
    Tensor A({1024}), B({1024});
    IRNodeKey I;
    {
      Variable i(0, 1024, 1);
      I = i.id;
      A(i) = B(i) + i;
    }
    auto root = prog.module_.GetRoot();
    auto i_loop = prog.module_.GetLoop(I);

    LoopVectorization::runPass(
        LoopVectorization::Arg::create(prog.module_.GetRoot(), i_loop, 8));
    // prog.IRGen();
    prog.GenerateC();
  }
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

    auto par_module = prog.module_.CreateSubSpace();
    auto root = par_module.GetRoot();
    auto i_loop = par_module.GetLoop(I);
    auto j_loop = par_module.GetLoop(J);
    auto k_loop = par_module.GetLoop(K);

    auto ret = ParallelizationAnalysisPass::runPass(
        ParallelizationAnalysisPass::Arg::create(root, i_loop));
  }

  {
    Program prog;
    Tensor A({1024}), B({1024});
    {
      Variable i(0, 1024, 1);
      A(i) = i;
      {
        Variable j(0, 1, 1);
        B(i + j) = i + j;
        A(i + 1) = A(i) + B(i + j);
      }
      B(i + 1) = B(i);
    }
    prog.GenerateC();
    SyncParallel::runPass(SyncParallel::Arg::create(prog.module_.GetRoot()));
    prog.GenerateC();
  }

  {
    Program prog;
    Tensor A({1024}), B({1024});
    {
      Variable i(0, 1024, 1);
      A(i) = i;
      {
        Variable j(0, 1, 1);
        B(i + j) = i + j;
        A(i + 1) = A(i) + B(i + j);
      }
      B(i + 1) = B(i);
    }
    prog.GenerateCuda();
  }

  {
    Program prog;
    Tensor A({1024});
    {
      Variable i(1, 1024, 1);
      {
        Variable j(Max(i, 10) - Min(i, 10), 1024, 1);
        A(j) = A(i);
      }
    }

    ConstantFoldingPass::runPass(std::shared_ptr<ConstantFoldingPass::Arg>(
        new ConstantFoldingPass::Arg(prog.module_.GetRoot())));
    prog.GenerateC();
  }

  {
    Program prog;
    Tensor A({1024, 1024}), B({1024, 1024}), C({1024, 1024});
    IRNodeKey I, J;
    {
      Variable i(0, (256) / (4), 1);
      I = i.id;
      {
        Variable j(0, 4, 1);
        J = j.id;
      }
    }
    auto root = prog.module_.GetRoot();
    auto i_loop = prog.module_.GetLoop(I).as<ForNode>()->looping_var_;
    auto j_loop = prog.module_.GetLoop(J).as<ForNode>()->looping_var_;
    // LoopReorder::runPass(LoopReorder::Arg::create(root, i_loop, j_loop));
    prog.GenerateC();
  }

  {
    Program prog;
    Tensor A({1024, 1024}), B({1024, 1024}), C({1024, 1024});
    IRNodeKey I;
    {
      Variable i(0, 1, 1);
      I = i.id;
      A(i) = i;
    }
    auto root = prog.module_.GetRoot();
    auto i_loop = prog.module_.GetLoop(I);
    LoopSplit::runPass(
        LoopSplit::Arg::create(prog.module_.GetRoot(), i_loop, 4));
    ConstantFoldingPass::runPass(
        ConstantFoldingPass::Arg::create(prog.module_.GetRoot()));
    DeadCodeElimination::runPass(
        DeadCodeElimination::Arg::create(prog.module_.GetRoot()));
    prog.GenerateC();
  }

  {
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

    prog.GenerateC();

    NormalizationPass::runPass(
        NormalizationPass::Arg::create(prog.module_.GetRoot()));
    prog.GenerateC();
    SyncParallel::runPass(SyncParallel::Arg::create(prog.module_.GetRoot()));
    // LoopParallel::runPass(LoopParallel::Arg::create(prog.module_.GetRoot()));
    prog.GenerateC();
  }

  return 0;
}