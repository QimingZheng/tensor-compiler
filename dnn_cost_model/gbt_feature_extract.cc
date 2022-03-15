#include "ir/ir.h"
#include "ir/ir_module.h"
#include "auto_scheduler/search_strategy/naive_random_search.h"
#include "auto_scheduler/cost_model/cost_model.h"
#include "common.h"
#include "lang/program.h"
#include "lang/expr.h"
#include "auto_scheduler/cost_model/arch_spec.h"

#include "pass/analysis/polyhedral_extraction.h"

using namespace polly;

class GBTFeatureExtract : public PolyhedralExtraction {
 public:
  // Computation features contains:
  // 1. number of memroy accesses per instance.
  // 2. number of arithmetic operations per instance.
  std::map<IRNodeKey, std::vector<int>> computation_features;
  // Loop features contains:
  // 1. Loop range
  // 2. parallelization
  // 3. vectorization (TODO)
  std::map<IRNodeKey, size_t> loop_feature_iter_number;
  std::map<IRNodeKey, std::map<IRNodeKey, size_t>>
      loop_feature_array_access_count;
  std::map<IRNodeKey, std::map<IRNodeKey, float>>
      loop_feature_array_reuse_ratio;

  std::map<IRNodeKey, std::vector<IRNodeKey>> tree_structure;
  std::map<IRNodeKey, int> node_id_map;

  IRNodeKey root = "root";

  std::set<IRNodeKey> array_keys;

  std::ofstream f;

  IRModule module_;

  GBTFeatureExtract(std::string json_output_file, IRModule module)
      : module_(module) {
    f.open(json_output_file, std::ios_base::app);
    visit(module_.GetRoot());
    f << "{";
    f << "\"nodes\":[";
    node_id_map[root] = 0;
    for (auto loop_node : loop_feature_iter_number) {
      f << "{";
      f << "\"feature\": [";
      f << log(loop_node.second);
      f << "]},";
      int id = node_id_map.size();
      node_id_map[loop_node.first] = id;
    }

    for (auto comp_node : loop_feature_array_access_count) {
      auto key = comp_node.first;
      f << "{";
      f << "\"feature\": [";
      // f << comp_node.second.size() << ",";
      size_t total_count = 0;
      for (auto ac : comp_node.second) {
        total_count += ac.second;
      }
      f << log(total_count) << ",";
      size_t actual_count = 0;
      for (auto ac : comp_node.second) {
        actual_count +=
            loop_feature_array_reuse_ratio[key][ac.first] * ac.second;
      }
      f << actual_count * 1.0 / total_count;
      f << "]},";
    }
    f << "],";

    f << "\"edges\":[";

    std::queue<IRNodeKey> q;
    q.push(root);
    while (!q.empty()) {
      auto head = q.front();
      q.pop();
      for (auto c : tree_structure[head]) {
        q.push(c);
        f << "[" << node_id_map[head] << "," << node_id_map[c] << "],";
      }
    }
    f << "],";

    f << "\"cost\":";

    f << log(CostModel().Evaluate(module_, ArchSpec(ArchSpec::ArchType::CPU),
                                  "undefined"));
    f << "}";
    f << "\n";
  }

  ~GBTFeatureExtract() { f.close(); }

  size_t DomainCount(IterDomain domain) {
    solver::context ctx;
    std::vector<std::string> iter_names;
    for (int i = 0; i < domain.iterations_.size(); i++) {
      iter_names.push_back(domain.iterations_[i].iterName_);
    }
    solver::IterSet polyhedral(ctx, "S", iter_names);

    for (int i = 0; i < domain.iterations_.size(); i++) {
      for (auto lb : domain.iterations_[i].lowerBounds_) {
        auto coeff = lb.coeffs;
        for (auto &it : coeff) {
          it.second = -it.second;
        }
        coeff[domain.iterations_[i].iterName_] = 1;
        auto lb_c = polyhedral.CreateInequality(coeff, -lb.constant);
        polyhedral.add_constraint(lb_c);
      }

      for (auto ub : domain.iterations_[i].upperBounds_) {
        auto coeff = ub.coeffs;
        coeff[domain.iterations_[i].iterName_] = -1;
        auto ub_c = polyhedral.CreateInequality(coeff, ub.constant);
        polyhedral.add_constraint(ub_c);
      }
    }
    return polyhedral.get_size();
  }

  void visitFor(ForHandle loop) override {
    tree_structure[loop->looping_var_.as<VarNode>()->id] =
        std::vector<IRNodeKey>();
    workspace.clear();
    loop->looping_var_.as<VarNode>()->min.accept(this);
    auto min_ws = workspace;
    auto mins_ = workspace.max_exprs.size() > 0
                     ? workspace.max_exprs
                     : std::vector<QuasiAffineExpr>{workspace.expr};
    workspace.clear();
    loop->looping_var_.as<VarNode>()->max.accept(this);
    auto max_ws = workspace;
    auto maxs_ = workspace.min_exprs.size() > 0
                     ? workspace.min_exprs
                     : std::vector<QuasiAffineExpr>{workspace.expr};

    for (auto &max_ : maxs_) {
      // max_ is a non-inclusive bound
      // max_.constant -= 1;
      max_.constant -= 1 * max_.divisor;
    }

    loops.push_back(
        Iteration(loop->looping_var_.as<VarNode>()->id, mins_, maxs_));

    IterDomain domain(loops);

    size_t count = DomainCount(domain);
    loop_feature_iter_number[loop->looping_var_.as<VarNode>()->id] = count;

    int before_size = model.statements_.size();
    for (int i = 0; i < loop->body.size(); i++) {
      if (loop->body[i].Type() == IRNodeType::FOR) {
        tree_structure[loop->looping_var_.as<VarNode>()->id].push_back(
            loop->body[i].as<ForNode>()->looping_var_.as<VarNode>()->id);
      }
      progContext.push_back(i);
      loop->body[i].accept(this);
      progContext.pop_back();
    }
    loops.pop_back();

    solver::context ctx;
    std::map<IRNodeKey, std::vector<solver::basic_map>> array_access_set;
    for (int i = before_size; i < model.statements_.size(); i++) {
      for (auto acc : model.statements_[i].accesses_) {
        auto array_name = acc.access.arrayName_;
        auto access_map =
            DataDependencyModel::BuildAccessMap(ctx, model.statements_[i], acc);
        if (array_access_set.find(array_name) != array_access_set.end()) {
          array_access_set[array_name].push_back(access_map.array_domain_);
        } else {
          array_access_set[array_name] = std::vector<solver::basic_map>();
          array_access_set[array_name].push_back(access_map.array_domain_);
        }
      }
    }

    for (auto it : array_access_set) {
      auto array_name = it.first;
      auto array_accessmaps = it.second;

      solver::set all_accessed_elem = solver::set(array_accessmaps[0].range());
      size_t all_access_coount = 0;
      for (auto accessmap : array_accessmaps) {
        auto domain_ = accessmap.domain();
        auto domain = solver::set(domain_);
        auto range_ = accessmap.range();
        auto range = solver::set(range_);
        all_accessed_elem = all_accessed_elem | range;
        all_access_coount += domain.get_size();
      }
      loop_feature_array_access_count[loop->looping_var_.as<VarNode>()->id]
          .insert(std::pair<IRNodeKey, size_t>(array_name,
                                               all_accessed_elem.get_size()));
      loop_feature_array_reuse_ratio[loop->looping_var_.as<VarNode>()->id]
          .insert(std::pair<IRNodeKey, float>(
              array_name,
              1 - (1.0f * all_accessed_elem.get_size()) / (all_access_coount)));
      array_keys.insert(array_name);
    }
  }

  void visitFunc(FuncHandle func) {
    tree_structure[root] = std::vector<IRNodeKey>();
    progContext.clear();
    for (int i = 0; i < func->body.size(); i++) {
      if (func->body[i].Type() == IRNodeType::FOR) {
        tree_structure[root].push_back(
            func->body[i].as<ForNode>()->looping_var_.as<VarNode>()->id);
      }
      progContext.push_back(i);
      func->body[i].accept(this);
      progContext.pop_back();
    }
  }

  int arith_count;
  int mem_access_count;
  int loop_range;
  bool parallelization;
  bool vectorization;
};

void CreateTrainingSample(std::string json_file, int transform_count) {
  Program prog;
  Tensor A({1024, 1024}), B({1024, 1024}), C({1024, 1024});
  {
    Variable i(0, 1024, 1);
    {
      Variable j(0, 1024, 1);
      {
        Variable k(0, 1024, 1);
        C(i, j) += A(i, k) * B(k, j);
      }
    }
  }

  prog.AutoTune("RandomSearch", transform_count);

  GBTFeatureExtract dataGen(json_file, prog.module_);
}

int main() {
  int dataset_sample_count = 1024;
  std::string json_file = "train.json";
  for (int i = 0; i < dataset_sample_count; i++) {
    std::cout << "creating " << i << "/" << dataset_sample_count << "\n";
    int random = 1 + (rand() % 32);
    CreateTrainingSample(json_file, random);
  }
  return 0;
}