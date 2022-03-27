#include "beam_search.h"

namespace polly {

void BeamSearchStrategy::RandomSearch(SearchNodeHandle parent,
                                      SearchNodeHandle child) {
  IRModule &module = child->module;
  // SearchNodeID search_node_id;
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
        tree->RegisterBranch(parent, child, "Fission");
        break;
      }
    }
    if (seed == 1) {
      // Fussion
      auto first_loop = GetRandomLoop(nodes);
      auto second_loop = GetRandomLoop(nodes);
      if (Mutator::Fussion(cloned_module.GetRoot(), first_loop, second_loop)) {
        module = cloned_module;
        tree->RegisterBranch(parent, child, "Fussion");
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
        tree->RegisterBranch(parent, child, "Split");

        break;
      }
    }
    if (seed == 3) {
      // Reorder
      auto first_loop = GetRandomLoop(nodes);
      auto second_loop = GetRandomLoop(nodes);
      if (Mutator::Reorder(cloned_module.GetRoot(), first_loop, second_loop)) {
        module = cloned_module;
        tree->RegisterBranch(parent, child, "Reorder");

        break;
      }
    }
    if (seed == 4) {
      // Parallelize
      if (Mutator::Parallelize(cloned_module.GetRoot())) {
        module = cloned_module;
        tree->RegisterBranch(parent, child, "Parallelize");
        break;
      }
    }
  }
}

IRModule BeamSearchStrategy::Search(IRModule module, ArchSpec spec,
                                    std::string program_name) {
  std::shared_ptr<SearchNode> root(new SearchNode(module));
  if (module.GetRoot() != NullIRHandle) {
    best_module_ = module;
    candidates.clear();
    candidates.push_back(root);
  }

  float progress = 0.0;
  int t = 0;
  best_performance_ = 1e9;
  int barWidth = 40;

  tree = new SearchTrees(root);

  while (t < search_budget_) {
    {
      // Display Progress Bar
      t++;
      progress = t * 1.0 / search_budget_;
      std::cout << "[";
      int pos = barWidth * progress;
      for (int i = 0; i < barWidth; ++i) {
        if (i < pos)
          std::cout << "=";
        else if (i == pos)
          std::cout << ">";
        else
          std::cout << " ";
      }
      std::cout << "] " << int(progress * 100.0) << "% " << best_performance_
                << " ms\r";
      std::cout.flush();
    }

    std::vector<SearchNodeHandle> childrens;

    for (int i = 0; i < candidates.size(); i++) {
      for (int j = 0; j < beam_search_width_; j++) {
        childrens.push_back(candidates[i]->derive());
      }
    }
    for (int i = 0; i < childrens.size(); i++) {
      RandomSearch(candidates[i / beam_search_width_], childrens[i]);
    }
    CostModel model;
    std::transform(childrens.begin(), childrens.end(), childrens.begin(),
                   [&](SearchNodeHandle &x) -> SearchNodeHandle {
                     //  SearchNode ret(x);
                     x->performance =
                         model.Evaluate(x->module, spec, program_name);
                     tree->RecordPerf(x, x->performance);
                     return x;
                   });

    std::sort(childrens.begin(), childrens.end(),
              [&](const SearchNodeHandle &x, const SearchNodeHandle &y) {
                return x->performance < y->performance;
              });

    if ((*childrens.begin())->performance < best_performance_) {
      best_module_ = (*childrens.begin())->module.CreateSubSpace();
      best_performance_ = (*childrens.begin())->performance;
    }
    candidates.clear();
    std::transform(childrens.begin(), childrens.end(),
                   std::back_inserter(candidates),
                   [](const SearchNodeHandle &x) { return x; });

    candidates.resize(candidate_size_);
  }
  Mutator::Parallelize(best_module_.GetRoot());
  SearchHistoryLog log(tree, "log.json");
  return best_module_;
}
}  // namespace polly