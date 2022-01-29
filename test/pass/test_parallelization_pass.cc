#include "gtest/gtest.h"

#include "lang/program.h"
#include "lang/expr.h"

#include "pass/parallelization/parallel_utils.h"
#include "pass/parallelization/sync_parallel.h"

using namespace polly;

TEST(SYNC_PARALLEL, SYNC_PARALLEL) {
  {
    Program prog;
    Tensor A({1024}), B({1024});
    {
      Variable i(0, 1024, 1);
      A(i) = i;  // s1
      {
        Variable j(0, 1, 1);
        B(i + j) = i + j;            // s2
        A(i + 1) = A(i) + B(i + j);  // s3
      }
      B(i + 1) = B(i);  // s4
    }

    auto top_level =
        prog.module_.GetRoot().as<FuncNode>()->body[0].as<ForNode>();

    auto s1 = top_level->body[0].as<AssignmentNode>()->id;
    auto s2 =
        top_level->body[1].as<ForNode>()->body[0].as<AssignmentNode>()->id;
    auto s3 =
        top_level->body[1].as<ForNode>()->body[1].as<AssignmentNode>()->id;
    auto s4 = top_level->body[2].as<AssignmentNode>()->id;
    SyncParallel::runPass(SyncParallel::Arg::create(prog.module_.GetRoot()));
    EXPECT_EQ(prog.module_.GetRoot().as<FuncNode>()->body.size(), 2);
    EXPECT_EQ(prog.module_.GetRoot()
                  .as<FuncNode>()
                  ->body[0]
                  .as<ForNode>()
                  ->body.size(),
              2);
    EXPECT_EQ(prog.module_.GetRoot()
                  .as<FuncNode>()
                  ->body[1]
                  .as<ForNode>()
                  ->body.size(),
              2);
    auto first_nesting_loop =
        prog.module_.GetRoot().as<FuncNode>()->body[0].as<ForNode>();
    auto second_nesting_loop =
        prog.module_.GetRoot().as<FuncNode>()->body[1].as<ForNode>();

    EXPECT_EQ(first_nesting_loop->body[0]
                  .as<ForNode>()
                  ->body[0]
                  .as<AssignmentNode>()
                  ->id,
              s2);
    EXPECT_EQ(first_nesting_loop->body[1].as<AssignmentNode>()->id, s4);
    EXPECT_EQ(second_nesting_loop->body[0].as<AssignmentNode>()->id, s1);
    EXPECT_EQ(second_nesting_loop->body[1]
                  .as<ForNode>()
                  ->body[0]
                  .as<AssignmentNode>()
                  ->id,
              s3);
  }
}