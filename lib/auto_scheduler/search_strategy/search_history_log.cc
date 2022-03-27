#include "search_history_log.h"

namespace polly {

SearchNodeKeyGen *SearchNodeKeyGen::generator = nullptr;

SearchNodeKeyGen *SearchNodeKeyGen::GetInstance() {
  if (generator == nullptr) generator = new SearchNodeKeyGen;
  return generator;
}

SearchTrees::SearchTrees(SearchNodeHandle root) : root(root) {
  trees[root] = {};
  node_descs[root] = "Root";
  node_perf[root] = __FLT_MAX__;
}

void SearchTrees::RegisterBranch(SearchNodeHandle parent,
                                 SearchNodeHandle child, std::string desc) {
  trees[parent].push_back(child);
  trees[child] = {};
  node_descs[child] = desc;
}

void SearchTrees::RecordPerf(SearchNodeHandle node, float perf) {
  node_perf[node] = perf;
}

// std::vector<SearchNodeID> SearchTrees::getRoots() { return trees[-1]; }

void SearchHistoryLog::dump_tree(
    std::unordered_map<SearchNodeHandle, std::vector<SearchNodeHandle>,
                       SearchNodeHasher> &trees,
    std::unordered_map<SearchNodeHandle, std::string, SearchNodeHasher>
        &node_descs,
    std::unordered_map<SearchNodeHandle, float, SearchNodeHasher> &node_perf,
    SearchNodeHandle cur) {
  //
  f << "{";
  f << "\"Id\": \"" << cur->id << "\",";
  f << "\"Perf\": " << node_perf[cur] << ",";
  f << "\"Operation\": \"" << node_descs[cur] << "\",";
  f << "\"Children\": [";
  for (int i = 0; i < trees[cur].size(); i++) {
    dump_tree(trees, node_descs, node_perf, trees[cur][i]);
    if (i != trees[cur].size() - 1) {
      f << ",";
    }
  }
  f << "]";
  f << "}";
}

void SearchHistoryLog::dump_node(std::vector<SearchNodeHandle> &nodes) {
  //
  f << "{";
  int i = 0;
  for (auto node : nodes) {
    f << "\"" << node->id << "\":";
    {
      CodeGenC codegen;
      codegen.visit(node->module.GetRoot());
      std::string program_text = codegen.oss.str();
      {
        std::regex e("\t");
        program_text = std::regex_replace(program_text, e, "\\t");
      }
      {
        std::regex e("\n");
        program_text = std::regex_replace(program_text, e, "\\n");
      }
      f << "\"" << program_text << "\"";
      if (i < nodes.size() - 1) {
        f << ",";
      }
    }
    i++;
  }
  f << "}";
}

}  // namespace polly