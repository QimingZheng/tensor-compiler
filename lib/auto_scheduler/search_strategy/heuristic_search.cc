#include "heuristic_search.h"
#include "auto_scheduler/cost_model/cost_model.h"
#include "auto_scheduler/cost_model/arch_spec.h"

namespace polly {

void HeuristicSearchStrategy::Expand(SearchNodeHandle &parent,
                                     SearchNodeHandle &child,
                                     std::string action) {
  IRModule &module = child->module;

  int max_trial = 10;
  while (max_trial--) {
    auto cloned_module = module.CreateSubSpace();
    auto nodes = cloned_module.GetIRNodes();
    if (action == "fission") {
      // Fission
      auto loop = GetRandomLoop(nodes);
      if (Mutator::Fission(cloned_module.GetRoot(), loop)) {
        module = cloned_module;
        tree->RegisterBranch(parent, child, "Fission");
        break;
      }
    }
    if (action == "fussion") {
      // Fussion
      auto first_loop = GetRandomLoop(nodes);
      auto second_loop = GetRandomLoop(nodes);
      if (Mutator::Fussion(cloned_module.GetRoot(), first_loop, second_loop)) {
        module = cloned_module;
        tree->RegisterBranch(parent, child, "Fussion");
        break;
      }
    }
    if (action == "split") {
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
    if (action == "reorder") {
      // Reorder
      auto first_loop = GetRandomLoop(nodes);
      auto second_loop = GetRandomLoop(nodes);
      if (Mutator::Reorder(cloned_module.GetRoot(), first_loop, second_loop)) {
        module = cloned_module;
        tree->RegisterBranch(parent, child, "Reorder");
        break;
      }
    }
    if (action == "parallel") {
      // Parallelize
      if (Mutator::Parallelize(cloned_module.GetRoot())) {
        module = cloned_module;
        tree->RegisterBranch(parent, child, "Parallelize");
        break;
      }
    }
  }
}

IRModule HeuristicSearchStrategy::Search(IRModule module, ArchSpec spec,
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

    std::vector<std::pair<std::string, float>> action_gammas = {
        {"fission", .7f}, {"fussion", .7f}, {"split", 1.2f},
        {"reorder", .9f}, {"parallel", 1.f},
    };

    float discount = .1f;

    for (int i = 0; i < candidates.size(); i++) {
      for (auto action : action_gammas) {
        childrens.push_back(candidates[i]->derive());
      }
    }
    for (int i = 0; i < childrens.size(); i++) {
      Expand(candidates[i / action_gammas.size()], childrens[i],
             action_gammas[i % action_gammas.size()].first);
    }
    CostModel model;
    std::transform(childrens.begin(), childrens.end(), childrens.begin(),
                   [&](SearchNodeHandle &x) -> SearchNodeHandle {
                     x->performance =
                         model.Evaluate(x->module, spec, program_name);
                     tree->RecordPerf(x, x->performance);
                     return x;
                   });

    float candidate_best_perf = __FLT_MAX__;
    auto candidate_best = (*childrens.begin());
    for (auto c : childrens) {
      if (candidate_best_perf > c->performance) {
        candidate_best_perf = std::min(candidate_best_perf, c->performance);
        candidate_best = c;
      }
    }

    for (int i = 0; i < childrens.size(); i++) {
      auto c = childrens[i];
      c->steps_counter += 1;
      c->accumulated_heuristics -=
          pow(action_gammas[i % action_gammas.size()].second, c->steps_counter);
      c->accumulated_costs = discount * (c->accumulated_costs) +
                             log(c->performance / candidate_best_perf);
    }

    if (candidate_best->performance < best_performance_) {
      best_module_ = candidate_best->module.CreateSubSpace();
      best_performance_ = candidate_best->performance;
    }

    candidates.clear();

    std::transform(childrens.begin(), childrens.end(),
                   std::back_inserter(candidates),
                   [](const SearchNodeHandle &x) { return x; });

    std::sort(childrens.begin(), childrens.end(),
              [&](const SearchNodeHandle &x, const SearchNodeHandle &y) {
                return x->accumulated_heuristics + x->accumulated_costs <
                       y->accumulated_heuristics + y->accumulated_costs;
              });

    candidates.resize(candidate_size_);
    {
      CodeGenC gen;
      std::cout << gen.genCode((*candidates.begin())->module.GetRoot(),
                               (*candidates.begin())->module.GetTensors(),
                               program_name);
    }
  }
  Mutator::Parallelize(best_module_.GetRoot());
  SearchHistoryLog log(tree, "log.json");
  return best_module_;
}

}  // namespace polly
