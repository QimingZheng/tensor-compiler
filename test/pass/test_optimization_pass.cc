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