#include "beam_search.h"

namespace polly {

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
      int seed = rand() % 3;
      switch (seed) {
        case 0: {
          childrens[i].first.RandomSplit();
          break;
        }
        case 1: {
          childrens[i].first.RandomReorder();
          break;
        }
        case 2: {
          childrens[i].first.RandomFuse();
          break;
        }
        default:
          throw std::runtime_error("out of random bound");
      }
    }
    CostModel model;
    std::transform(
        childrens.begin(), childrens.end(), childrens.begin(),
        [&](const std::pair<IRModule, float> &x) -> std::pair<IRModule, float> {
          return {x.first, model.Evaluate(x.first)};
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
  return best_module_;
}
}  // namespace polly