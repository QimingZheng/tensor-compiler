#include "ir/ir.h"
#include "ir/ir_module.h"
#include "auto_scheduler/search_strategy/naive_random_search.h"
#include "auto_scheduler/cost_model/cost_model.h"
#include "common.h"
#include "lang/program.h"
#include "lang/expr.h"
#include "auto_scheduler/cost_model/arch_spec.h"

using namespace polly;

class TrainingDataGen : public IRNotImplementedVisitor {
 public:
  // Computation features contains:
  // 1. number of memroy accesses per instance.
  // 2. number of arithmetic operations per instance.
  std::map<IRNodeKey, std::vector<int>> computation_features;
  // Loop features contains:
  // 1. Loop range
  // 2. parallelization
  // 3. vectorization (TODO)
  std::map<IRNodeKey, std::vector<int>> loop_features;

  std::map<IRNodeKey, int> computation_index_dict;

  std::map<IRNodeKey, int> loop_index_dict;

  std::vector<int> computations;

  std::ofstream f;

  IRModule module_;

  TrainingDataGen(std::string json_output_file, IRModule module)
      : module_(module) {
    f.open(json_output_file, std::ios_base::app);
    visit(module_.GetRoot());
    f << "\n";
  }

  ~TrainingDataGen() { f.close(); }

  void visitInt(IntHandle int_expr) override {
    arith_count = 0;
    mem_access_count = 0;
  }
  void visitFloat(FloatHandle float_expr) override {
    arith_count = 0;
    mem_access_count = 0;
  }
  void visitAdd(AddHandle add) override {
    int arith = 0;
    int mem_access = 0;
    add->lhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    add->rhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    arith_count = arith;
    mem_access_count = mem_access;
  }
  void visitSub(SubHandle sub) override {
    int arith = 0;
    int mem_access = 0;
    sub->lhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    sub->rhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    arith_count = arith;
    mem_access_count = mem_access;
  }
  void visitMul(MulHandle mul) override {
    int arith = 0;
    int mem_access = 0;
    mul->lhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    mul->rhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    arith_count = arith;
    mem_access_count = mem_access;
  }
  void visitDiv(DivHandle div) override {
    int arith = 0;
    int mem_access = 0;
    div->lhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    div->rhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    arith_count = arith;
    mem_access_count = mem_access;
  }
  void visitMod(ModHandle mod) override {
    int arith = 0;
    int mem_access = 0;
    mod->lhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    mod->rhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    arith_count = arith;
    mem_access_count = mem_access;
  }
  void visitVar(VarHandle var) override {
    arith_count = 0;
    mem_access_count = 0;
  }
  void visitAccess(AccessHandle access) override {
    arith_count = 0;
    mem_access_count = 1;
  }
  void visitAssign(AssignmentHandle assign) override {
    int arith = 0;
    int mem_access = 0;
    assign->lhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    assign->rhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    computation_features[assign->id] = {arith, mem_access};
    auto cnt = computation_index_dict.size();
    computation_index_dict[assign->id] = cnt;
    computations.push_back(computation_index_dict[assign->id]);
  }
  void visitTensor(TensorHandle tensor) override {
    // Pass
  }
  void visitFor(ForHandle loop) override {
    auto comps = computations;
    computations.clear();
    f << "{";
    f << "\"loop_index\": ";
    auto cnt = loop_index_dict.size();
    auto looping_var = loop->looping_var_.as<VarNode>();
    loop_index_dict[looping_var->id] = cnt;
    loop_features[looping_var->id] = {
        loop->annotation.parallelization, loop->annotation.vectorization,
        (looping_var->max.Type() == IRNodeType::INT)
            ? looping_var->max.as<IntNode>()->value
            : 0};
    f << loop_index_dict[looping_var->id] << ",";
    f << "\"child_list\": [";
    for (int i = 0; i < loop->body.size(); i++) {
      loop->body[i].accept(this);
    }
    f << "],";
    f << "\"has_comps\":";
    if (computations.size() > 0) {
      f << "true,";
      f << "\"computations_indices\":";
      f << "[";
      for (int t = 0; t < computations.size(); t++) {
        if (t) f << ", ";
        f << computations[t];
      }
      f << "]";
    } else {
      f << "false";
    }

    f << "}";
    computations = comps;
  }
  void visitPrint(PrintHandle print) override {
    print->print.accept(this);
    computation_features[print->id] = {arith_count, mem_access_count};
  }
  void visitFunc(FuncHandle func) override {
    loop_features["root"] = {0, 0, 1};
    loop_index_dict["root"] = 0;

    f << "{";
    f << "\"loop_index\": 0,";
    f << "\"child_list\": [";
    for (int i = 0; i < func->body.size(); i++) {
      func->body[i].accept(this);
    }
    f << "],";
    f << "\"has_comps\":";
    if (computations.size() > 0) {
      f << "true,";
      f << "\"computations_indices\":";
      f << "[";
      for (int t = 0; t < computations.size(); t++) {
        if (t) f << ", ";
        f << computations[t];
      }
      f << "],";
    } else {
      f << "false,";
    }

    f << "\"loop_feature_tensors\": [";
    std::vector<std::vector<int>> features;
    features.resize(loop_features.size());
    for (auto it : loop_features) {
      features[loop_index_dict[it.first]] = it.second;
    }
    for (int i = 0; i < features.size(); i++) {
      if (i) f << ", ";
      f << "[";
      for (int j = 0; j < features[i].size(); j++) {
        if (j) f << ", ";
        f << features[i][j];
      }
      f << "]";
    }

    f << "],";

    f << "\"computation_feature_tensors\": [";
    features.resize(computation_features.size());
    for (auto it : computation_features) {
      features[computation_index_dict[it.first]] = it.second;
    }
    for (int i = 0; i < features.size(); i++) {
      if (i) f << ", ";
      f << "[";
      for (int j = 0; j < features[i].size(); j++) {
        if (j) f << ", ";
        f << features[i][j];
      }
      f << "]";
    }
    f << "],";

    f << "\"label\": ";
    f << CostModel().Evaluate(module_, ArchSpec(ArchSpec::ArchType::CPU),
                              "undefined");
    f << "}";
  }

  void visitMin(MinHandle min) override {
    int arith = 0;
    int mem_access = 0;
    min->lhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    min->rhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    arith_count = arith;
    mem_access_count = mem_access;
  }
  void visitMax(MaxHandle max) override {
    int arith = 0;
    int mem_access = 0;
    max->lhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    max->rhs.accept(this);
    arith += arith_count;
    mem_access += mem_access_count;
    arith_count = arith;
    mem_access_count = mem_access;
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

  TrainingDataGen dataGen(json_file, prog.module_);
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