#include "monte_carlo_search.h"
#include "naive_random_search.h"

namespace polly {

IRModule MonteCarloSearchStrategy::Search(IRModule module, ArchSpec spec,
                                          std::string program_name) {
  std::vector<IRModule> candidates;
  candidates.push_back(module);
  int t = 0, search_trials;

  while (t < search_trials) {
    t++;
    std::vector<std::pair<IRModule, float>> seeds;
    for (auto &m : candidates) {
      for (int i = 0; i < expanding_size; i++) {
        seeds.push_back({Expand(m, spec, program_name), .0});
      }
    }
    for (auto &m : seeds) {
      CostModel costmodel;
      m.second = costmodel.Evaluate(m.first, spec, program_name);
    }
    std::sort(seeds.begin(), seeds.end(),
              [&](const std::pair<IRModule, float> &x,
                  const std::pair<IRModule, float> &y) {
                return x.second < y.second;
              });
    seeds.resize(candidate_size);
    for (auto &m : seeds) {
      CostModel costmodel;
      m.second = costmodel.Evaluate(m.first, spec, program_name);
    }
    std::sort(seeds.begin(), seeds.end(),
              [&](const std::pair<IRModule, float> &x,
                  const std::pair<IRModule, float> &y) {
                return x.second < y.second;
              });
    candidates.clear();
    for (int i = 0; i < std::min(candidate_size, seeds.size()); i++) {
      candidates.push_back(seeds[i].first);
    }
  }
  return candidates[0];
}

IRModule MonteCarloSearchStrategy::Expand(IRModule module, ArchSpec spec,
                                          std::string program_name) {
  //
  return RandomSearchStrategy().Search(module, spec, program_name);
}

}  // namespace polly