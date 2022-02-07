#include "gtest/gtest.h"

#include "lang/program.h"
#include "lang/expr.h"

#include "pass/optimization/constant_folding.h"

using namespace polly;

TEST(CONSTANT_FOLDING_PASS, CONSTANT_FOLDING) {
  {
    Program prog;
    Tensor A({1024});
    {
      Variable i(1, 1024, 1);
      A(i) = i + 2 - 0 * A(i - 0);
    }

    auto nodes = prog.module_.GetIRNodes();
    EXPECT_EQ(nodes.find(IntNode::make(0)) == nodes.end(), false);
    EXPECT_EQ(nodes.find(IntNode::make(2)) == nodes.end(), false);
    ConstantFoldingPass::runPass(std::shared_ptr<ConstantFoldingPass::Arg>(
        new ConstantFoldingPass::Arg(prog.module_.GetRoot())));
    nodes = prog.module_.GetIRNodes();
    EXPECT_EQ(nodes.find(IntNode::make(0)) == nodes.end(), true);
    EXPECT_EQ(nodes.find(IntNode::make(2)) == nodes.end(), false);
  }
}

TEST(CONSTANT_FOLDING_PASS, MIN_MAX_FOLDING) {
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
    auto nodes = prog.module_.GetIRNodes();
    EXPECT_EQ(nodes.find(IntNode::make(0)) == nodes.end(), false);
  }
}

TEST(DEAD_CODE_ELIMINATION, DEAD_CODE_ELIMINATION) {
  {
    Program prog;
    Tensor A({1024, 1024});
    {
      Variable i(0, 1024, 1);
      {
        Variable j(0, 1024 / 32, 1);
        A(i, j) = i + j;
      }
      {
        Variable j(1024, 1024, 1);
        A(i, j) = i + j;
      }
    }

    EXPECT_EQ(prog.module_.GetRoot()
                  .as<FuncNode>()
                  ->body[0]
                  .as<ForNode>()
                  ->body.size(),
              2);
    DeadCodeElimination::runPass(
        DeadCodeElimination::Arg::create(prog.module_.GetRoot()));
    EXPECT_EQ(prog.module_.GetRoot()
                  .as<FuncNode>()
                  ->body[0]
                  .as<ForNode>()
                  ->body.size(),
              1);
  }
}