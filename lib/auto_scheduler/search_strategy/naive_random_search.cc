#include "naive_random_search.h"

namespace polly {

IRModule RandomSearchStrategy::Search(IRModule module) {
  while (search_budget_--) {
    int seed = rand() % 5;
    int max_trial = 10;
    while (max_trial--) {
      auto cloned_module = module.CreateSubSpace();
      auto nodes = cloned_module.GetIRNodes();
      if (seed == 0) {
        // Fission
        auto loop = GetRandomLoop(nodes);
        if (Mutator::Fission(cloned_module.GetRoot(), loop)) {
          module = cloned_module;
          break;
        }
      }
      if (seed == 1) {
        // Fussion
        auto first_loop = GetRandomLoop(nodes);
        auto second_loop = GetRandomLoop(nodes);
        if (Mutator::Fussion(cloned_module.GetRoot(), first_loop,
                             second_loop)) {
          module = cloned_module;
          break;
        }
      }
      if (seed == 2) {
        // Split
        auto toSplit = GetRandomLoop(nodes);
        if (Mutator::Split(cloned_module.GetRoot(), toSplit, 4)) {
          module = cloned_module;
          break;
        }
      }
      if (seed == 3) {
        // Reorder
        auto first_loop = GetRandomLoop(nodes);
        auto second_loop = GetRandomLoop(nodes);
        if (Mutator::Reorder(cloned_module.GetRoot(), first_loop,
                             second_loop)) {
          module = cloned_module;
          break;
        }
      }
      if (seed == 4) {
        // Parallelize
        if (Mutator::Parallelize(cloned_module.GetRoot())) {
          module = cloned_module;
          break;
        }
      }
      // if (seed = 5) {
      //   // Unroll
      //   if (Mutator::Unroll(cloned_module.GetRoot())) {
      //     module = cloned_module;
      //     break;
      //   }
      // }
    }
  }
  return module;
}
}  // namespace polly