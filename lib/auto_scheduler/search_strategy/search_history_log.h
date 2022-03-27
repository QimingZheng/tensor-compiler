#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_module.h"
#include "codegen/codegen.h"

namespace polly {

typedef int64_t SearchNodeID;

class SearchNodeKeyGen {
  SearchNodeKeyGen() {}

  SearchNodeID id = 0;

 protected:
  static SearchNodeKeyGen *generator;

 public:
  static SearchNodeKeyGen *GetInstance();

  SearchNodeID YieldID() { return id++; }
};

class SearchNode;

typedef std::shared_ptr<SearchNode> SearchNodeHandle;

class SearchNode {
 private:
  SearchNode(IRModule module, SearchNodeID id, float performance,
             std::string description, int step_count,
             float accumulated_heuristics, float accumulated_costs)
      : module(module),
        id(id),
        performance(performance),
        description(description),
        accumulated_heuristics(accumulated_heuristics),
        accumulated_costs(accumulated_costs),
        steps_counter(step_count) {}

 public:
  SearchNode(const SearchNode &other)
      : module(other.module),
        id(other.id),
        performance(other.performance),
        description(other.description),
        accumulated_heuristics(other.accumulated_heuristics),
        accumulated_costs(other.accumulated_costs),
        steps_counter(other.steps_counter) {}

  // Used for initializing the search root.
  SearchNode(IRModule module) : module(module) {
    id = SearchNodeKeyGen::GetInstance()->YieldID();
    performance = __FLT_MAX__;
    description = "Not Specified";
    accumulated_heuristics = 0.0f;
    accumulated_costs = 0.0f;
    steps_counter = 0;
  }

  SearchNode &operator=(const SearchNode &other) {
    module = other.module;
    id = other.id;
    performance = other.performance;
    description = other.description;
    accumulated_heuristics = other.accumulated_heuristics;
    accumulated_costs = other.accumulated_costs;
    steps_counter = other.steps_counter;
  }
  SearchNode(SearchNode &&other)
      : module(other.module),
        id(other.id),
        performance(other.performance),
        description(other.description),
        accumulated_heuristics(other.accumulated_heuristics),
        accumulated_costs(other.accumulated_costs),
        steps_counter(other.steps_counter) {}

  SearchNode &operator=(SearchNode &&other) {
    module = other.module;
    id = other.id;
    performance = other.performance;
    description = other.description;
    accumulated_heuristics = other.accumulated_heuristics;
    accumulated_costs = other.accumulated_costs;
    steps_counter = other.steps_counter;
  }

  SearchNodeHandle derive() {
    auto mod = module.CreateSubSpace();
    return std::shared_ptr<SearchNode>(
        new SearchNode(mod, SearchNodeKeyGen::GetInstance()->YieldID(),
                       __FLT_MAX__, "Not Specified", steps_counter,
                       accumulated_heuristics, accumulated_costs));
  }

  bool operator==(const SearchNode &other) const { return id == other.id; }

  IRModule module;
  SearchNodeID id;
  float performance;
  std::string description;

  /// The following fields are used by the heuristic search method.
  float accumulated_heuristics;
  float accumulated_costs;
  int steps_counter;
};

struct SearchNodeHasher {
  std::size_t operator()(const SearchNodeHandle &k) const {
    using std::hash;
    using std::size_t;
    using std::string;

    return hash<SearchNodeID>()(k->id);
  }
};

class SearchTrees {
 public:
  SearchTrees(SearchNodeHandle root);

  void RegisterBranch(SearchNodeHandle parent, SearchNodeHandle child,
                      std::string desc = "");
  void RecordPerf(SearchNodeHandle node, float perf);
  // std::vector<SearchNode> getRoots();

  // Root id = -1;
  std::unordered_map<SearchNodeHandle, std::vector<SearchNodeHandle>,
                     SearchNodeHasher>
      trees;
  std::unordered_map<SearchNodeHandle, std::string, SearchNodeHasher>
      node_descs;
  std::unordered_map<SearchNodeHandle, float, SearchNodeHasher> node_perf;
  SearchNodeHandle root;
};

// A logger module that logs the whole history of the program ir searching
// process.
class SearchHistoryLog {
 public:
  SearchHistoryLog(SearchTrees *history, std::string log_path) {
    std::string json = log_path;
    f.open(json);
    f << "{";
    f << "\"Tree\": ";
    dump_tree(history->trees, history->node_descs, history->node_perf,
              history->root);
    f << ",";
    f << "\"Nodes\": ";
    std::vector<SearchNodeHandle> nodes;
    for (auto it : history->trees) nodes.push_back(it.first);
    dump_node(nodes);
    f << "}";
    f.close();
  }

  std::ofstream f;

 private:
  void dump_tree(
      std::unordered_map<SearchNodeHandle, std::vector<SearchNodeHandle>,
                         SearchNodeHasher> &trees,
      std::unordered_map<SearchNodeHandle, std::string, SearchNodeHasher>
          &node_descs,
      std::unordered_map<SearchNodeHandle, float, SearchNodeHasher> &node_perf,
      SearchNodeHandle cur);

  void dump_node(std::vector<SearchNodeHandle> &nodes);
};

}  // namespace polly