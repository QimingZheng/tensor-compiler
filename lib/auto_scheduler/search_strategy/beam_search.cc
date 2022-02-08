#include "beam_search.h"

namespace polly {

void BeamSearchStrategy::RandomSearch(IRModule &module) {
  int seed = rand() % 4;
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
      if (Mutator::Fussion(cloned_module.GetRoot(), first_loop, second_loop)) {
        module = cloned_module;
        break;
      }
    }
    if (seed == 2) {
      // Split
      auto toSplit = GetRandomLoop(nodes);

      std::vector<int> divisors = {128, 64, 32, 16, 8, 4, 2};
      auto rng = std::default_random_engine{rand()};
      std::shuffle(divisors.begin(), divisors.end(), rng);

      if (Mutator::Split(cloned_module.GetRoot(), toSplit, divisors[0])) {
        module = cloned_module;
        break;
      }
    }
    if (seed == 3) {
      // Reorder
      auto first_loop = GetRandomLoop(nodes);
      auto second_loop = GetRandomLoop(nodes);
      if (Mutator::Reorder(cloned_module.GetRoot(), first_loop, second_loop)) {
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
  }
}

IRModule BeamSearchStrategy::Search(IRModule module) {
  if (module.GetRoot() != NullIRHandle) {
    best_module_ = module;
    candidates.clear();
    candidates.push_back(best_module_);
  }
  while (search_budget_--) {
    std::vector<std::pair<IRModule, float>> childrens;
    for (int i = 0; i < candidates.size(); i++) {
      for (int j = 0; j < beam_search_width_; j++) {
        childrens.push_back({candidates[i].CreateSubSpace(), 100.0});
      }
    }
    for (int i = 0; i < childrens.size(); i++) {
      RandomSearch(childrens[i].first);
      // int seed = 1 + rand() % 1;
      // switch (seed) {
      //   // case 0: {
      //   //   childrens[i].first.RandomSplit();
      //   //   break;
      //   // }
      //   case 1: {
      //     childrens[i].first.RandomReorder();
      //     break;
      //   }
      //   // case 2: {
      //   //   childrens[i].first.RandomFuse();
      //   //   break;
      //   // }
      //   default:
      //     throw std::runtime_error("out of random bound");
      // }
    }
    CostModel model;
    std::transform(
        childrens.begin(), childrens.end(), childrens.begin(),
        [&](const std::pair<IRModule, float> &x) -> std::pair<IRModule, float> {
          IRModule ori = x.first;
          auto cloned = ori.CreateSubSpace();

          Mutator::Parallelize(cloned.GetRoot());
          return {x.first, model.Evaluate(cloned)};
          // return {x.first, model.Evaluate(x.first)};
        });
    std::sort(childrens.begin(), childrens.end(),
              [&](const std::pair<IRModule, float> &x,
                  const std::pair<IRModule, float> &y) {
                return x.second < y.second;
              });
    if (childrens.begin()->second < best_performance_) {
      best_module_ = childrens.begin()->first.CreateSubSpace();
    }
    candidates.clear();
    std::transform(childrens.begin(), childrens.end(),
                   std::back_inserter(candidates),
                   [](const std::pair<IRModule, float> &x) { return x.first; });
    candidates.resize(candidate_size_);
  }
  Mutator::Parallelize(best_module_.GetRoot());
  return best_module_;
}
}  // namespace polly