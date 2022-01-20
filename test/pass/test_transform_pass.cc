#include "gtest/gtest.h"

#include "lang/program.h"
#include "lang/expr.h"

#include "pass/transform/fission.h"
#include "pass/transform/fussion.h"
#include "pass/transform/normalization.h"
#include "pass/transform/reorder.h"
#include "pass/transform/split.h"
#include "pass/transform/unroll.h"
#include "pass/transform/vectorization.h"

using namespace polly;

TEST(TRANSFORM_PASS, FUSSION) {
  {
    Program prog;
    Tensor A({1024}), B({1024});
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

    IRHandle ori = prog.module_.CreateSubSpace().GetRoot();
    EXPECT_EQ(ori.as<FuncNode>()->body.size(), 2);
    IRHandle first_loop = prog.module_.GetLoop(I);
    IRHandle second_loop = prog.module_.GetLoop(J);
    FussionTransform::runPass(FussionTransform::Arg::create(
        prog.module_.GetRoot(), first_loop, second_loop));
    IRHandle transformed = prog.module_.GetRoot();
    EXPECT_EQ(transformed.as<FuncNode>()->body.size(), 1);
    EXPECT_EQ(transformed.as<FuncNode>()->body[0].as<ForNode>()->body.size(),
              2);
  }
}

TEST(TRANSFORM_PASS, FISSION) {
  {
    Program prog;
    Tensor A({1024}), B({1024});
    IRNodeKey I;
    {
      Variable i(0, 1024, 1);
      I = i.id;
      A(i) = i * 2;
      B(i) = A(i) + i * 3;
    }

    IRHandle ori = prog.module_.CreateSubSpace().GetRoot();
    EXPECT_EQ(ori.as<FuncNode>()->body.size(), 1);
    IRHandle loop = prog.module_.GetLoop(I);
    FissionTransform::runPass(
        FissionTransform::Arg::create(prog.module_.GetRoot(), loop));
    IRHandle transformed = prog.module_.GetRoot();
    EXPECT_EQ(transformed.as<FuncNode>()->body.size(), 2);
    EXPECT_EQ(transformed.as<FuncNode>()->body[0].as<ForNode>()->body.size(),
              1);
    EXPECT_EQ(transformed.as<FuncNode>()->body[1].as<ForNode>()->body.size(),
              1);
  }
}

TEST(TRANSFORM_PASS, NORMALIZATION) {
  {
    Program prog;
    Tensor A({1024, 1024}), B({1024, 1024});
    {
      Variable i(6, 1024, 2);
      {
        Variable j(16, i + 1024, 4);
        A(i, j) = B(i, j);
      }
    }
    IRHandle ori = prog.module_.CreateSubSpace().GetRoot();
    NormalizationPass::runPass(
        NormalizationPass::Arg::create(prog.module_.GetRoot()));
    ConstantFoldingPass::runPass(
        ConstantFoldingPass::Arg::create(prog.module_.GetRoot()));
    IRHandle transformed = prog.module_.GetRoot();
    auto outter_loop = prog.module_.GetRoot().as<FuncNode>()->body[0];
    auto inner_loop = outter_loop.as<ForNode>()->body[0];
    EXPECT_EQ(outter_loop.as<ForNode>()->looping_var_.as<VarNode>()->min ==
                  IntNode::make(0),
              true);
    EXPECT_EQ(
        outter_loop.as<ForNode>()->looping_var_.as<VarNode>()->increment ==
            IntNode::make(1),
        true);
    EXPECT_EQ(inner_loop.as<ForNode>()->looping_var_.as<VarNode>()->min ==
                  IntNode::make(0),
              true);
    EXPECT_EQ(inner_loop.as<ForNode>()->looping_var_.as<VarNode>()->increment ==
                  IntNode::make(1),
              true);
  }
}

TEST(TRANSFORM_PASS, REORDER) {
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
    auto i_loop_var = prog.module_.GetLoop(I).as<ForNode>()->looping_var_;
    auto j_loop_var = prog.module_.GetLoop(J).as<ForNode>()->looping_var_;
    LoopReorder::runPass(
        LoopReorder::Arg::create(root, i_loop_var, j_loop_var));
    ConstantFoldingPass::runPass(
        ConstantFoldingPass::Arg::create(prog.module_.GetRoot()));
    auto new_outter_loop_var = prog.module_.GetRoot()
                                   .as<FuncNode>()
                                   ->body[0]
                                   .as<ForNode>()
                                   ->looping_var_;
    auto new_inner_loop_var = prog.module_.GetRoot()
                                  .as<FuncNode>()
                                  ->body[0]
                                  .as<ForNode>()
                                  ->body[0]
                                  .as<ForNode>()
                                  ->looping_var_;
    EXPECT_EQ(new_outter_loop_var == j_loop_var, true);
    EXPECT_EQ(new_outter_loop_var.as<VarNode>()->min == IntNode::make(0), true);
    EXPECT_EQ(new_outter_loop_var.as<VarNode>()->max == IntNode::make(1024),
              true);
    EXPECT_EQ(new_inner_loop_var == i_loop_var, true);
    EXPECT_EQ(new_inner_loop_var.as<VarNode>()->min == new_outter_loop_var,
              true);
    EXPECT_EQ(new_inner_loop_var.as<VarNode>()->max == IntNode::make(1024),
              true);
  }
}

TEST(TRANSFORM_PASS, SPLIT) {
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
    EXPECT_EQ(prog.module_.GetRoot()
                  .as<FuncNode>()
                  ->body[0]
                  .as<ForNode>()
                  ->body[0]
                  .Type(),
              IRNodeType::FOR);
  }
}

TEST(TRANSFORM_PASS, UNROLL) {
  {
    Program prog;
    Tensor A({1024});
    IRNodeKey I;
    {
      Variable i(0, 16, 1);
      I = i.id;
      A(i) = i + A(i);
      A(i) = A(i) - 10;
    }
    auto root = prog.module_.GetRoot();
    auto i_loop_var = prog.module_.GetLoop(I);
    LoopUnroll::runPass(LoopUnroll::Arg::create(i_loop_var));
    EXPECT_EQ(prog.module_.GetRoot()
                  .as<FuncNode>()
                  ->body[0]
                  .as<ForNode>()
                  ->body.size(),
              32);
  }
}

TEST(TRANSFORM_PASS, VECTORIZATION) {}