#include "sync_parallel.h"

namespace polly {

PassRetHandle SyncParallel::runPass(PassArgHandle arg) {
  SyncParallel sync_par(PassArg::as<SyncParallel::Arg>(arg)->program);
  return SyncParallel::Ret::create();
}

class SyncParallelHelper : public IRNotImplementedVisitor {
 public:
  IRHandle loop_, replace_loop_;

  SyncParallelHelper(IRHandle loop, IRHandle replace_loop)
      : loop_(loop), replace_loop_(replace_loop) {}

  IRHandle replace_if_match(IRHandle origin) {
    origin.accept(this);
    if (origin.equals(loop_.as<ForNode>()->looping_var_)) {
      return replace_loop_.as<ForNode>()->looping_var_;
    } else {
      return origin;
    }
  }

  void visitInt(IntHandle int_expr) override {
    /// Pass
  }
  void visitFloat(FloatHandle float_expr) override {
    /// Pass
  }

  void visitAdd(AddHandle add) override {
    add->lhs = replace_if_match(add->lhs);
    add->rhs = replace_if_match(add->rhs);
  }

  void visitSub(SubHandle sub) override {
    sub->lhs = replace_if_match(sub->lhs);
    sub->rhs = replace_if_match(sub->rhs);
  }

  void visitMul(MulHandle mul) override {
    mul->lhs = replace_if_match(mul->lhs);
    mul->rhs = replace_if_match(mul->rhs);
  }

  void visitDiv(DivHandle div) override {
    div->lhs = replace_if_match(div->lhs);
    div->rhs = replace_if_match(div->rhs);
  }

  void visitMod(ModHandle mod) override {
    mod->lhs = replace_if_match(mod->lhs);
    mod->rhs = replace_if_match(mod->rhs);
  }

  void visitVar(VarHandle var) override {
    var->min = replace_if_match(var->min);
    var->max = replace_if_match(var->max);
    var->increment = replace_if_match(var->increment);
  }

  void visitAccess(AccessHandle access) override {
    for (int i = 0; i < access->indices.size(); i++) {
      access->indices[i] = replace_if_match(access->indices[i]);
    }
  }

  void visitAssign(AssignmentHandle assign) override {
    assign->lhs = replace_if_match(assign->lhs);
    assign->rhs = replace_if_match(assign->rhs);
  }

  void visitTensor(TensorHandle tensor) override {
    /// Pass
  }

  void visitVal(ValHandle val) override {
    /// Pass
  }

  void visitConst(ConstHandle con) override {
    // Pass
  }

  void visitPrint(PrintHandle print) override {
    /// TODO
  }

  void visitFor(ForHandle loop) override {
    loop->looping_var_ = replace_if_match(loop->looping_var_);
    for (int i = 0; i < loop->body.size(); i++) {
      loop->body[i] = replace_if_match(loop->body[i]);
    }
  }

  void visitMin(MinHandle min) override {
    min->lhs = replace_if_match(min->lhs);
    min->rhs = replace_if_match(min->rhs);
  }
  void visitMax(MaxHandle max) override {
    max->lhs = replace_if_match(max->lhs);
    max->rhs = replace_if_match(max->rhs);
  }
};

std::vector<IRHandle> SyncParallel::Adjust(
    std::vector<IRHandle> enclosing_looping_vars, IRHandle loop) {
  using namespace internal;
  enclosing_looping_vars.push_back(loop.as<ForNode>()->looping_var_);
  solver::context ctx;
  std::vector<PolyhedralModel> models;
  std::vector<NodeHandle> nodes;
  TarjanSCC scc;
  int cnt = 0;
  std::vector<int> prog_ctx(enclosing_looping_vars.size(), 0);
  for (auto it : loop.as<ForNode>()->body) {
    prog_ctx.push_back(cnt);
    auto dep_model =
        PolyhedralExtraction(enclosing_looping_vars, prog_ctx, it).model;
    prog_ctx.pop_back();
    models.push_back(dep_model);
    nodes.push_back(Node::create(std::to_string(cnt++)));
    scc.AddNode(nodes.back());
  }

  // pair-wise program-dependency-edge
  for (int i = 0; i < models.size(); i++) {
    for (int j = 0; j < models.size(); j++) {
      if (i == j) continue;
      auto first_model = models[i];
      auto second_model = models[j];
      auto union_model = first_model + second_model;
      // auto dep = DataDependencyModel(ctx, union_model);
      prog_ctx.pop_back();
      auto dep = DataDependencyModel(ctx, prog_ctx, union_model);
      prog_ctx.push_back(0);

      // [... j ...] -> [... i ...]
      if (!((solver::ScheduleMap::ConcurrentMap(ctx, 2 * prog_ctx.size(), j, i,
                                                dep.GetDepth()) &
             dep.WAW.dependency)
                .empty()) ||
          !((solver::ScheduleMap::ConcurrentMap(ctx, 2 * prog_ctx.size(), j, i,
                                                dep.GetDepth()) &
             dep.RAW.dependency)
                .empty()) ||
          !((solver::ScheduleMap::ConcurrentMap(ctx, 2 * prog_ctx.size(), j, i,
                                                dep.GetDepth()) &
             dep.WAR.dependency)
                .empty())) {
        scc.AddEdge(nodes[i], nodes[j]);
      }
    }
  }
  scc.FindSCC();

  std::map<int, std::vector<NodeHandle>> components;
  for (int i = 0; i < nodes.size(); i++) {
    components[nodes[i]->component_id].push_back(nodes[i]);
  }

  std::map<int, NodeHandle> component_nodes;
  TopologicalSort topo;
  for (auto it : components) {
    component_nodes[it.first] = Node::create(std::to_string(it.first));
    topo.AddNode(component_nodes[it.first]);
  }

  for (auto i : components) {
    for (auto j : components) {
      if (i.first == j.first) continue;
      bool found = false;
      for (auto i_it : i.second) {
        if (found) break;
        for (auto j_it : j.second) {
          if (i_it->outgoings.find(j_it->id) != i_it->outgoings.end()) {
            found = true;
            break;
          }
        }
      }
      if (found) {
        topo.AddEdge(component_nodes[i.first], component_nodes[j.first]);
      }
    }
  }

  auto order = topo.Sort();

  std::vector<IRHandle> ret;

  int counter = 0;

  for (int c = 0; c < order.size(); c++) {
    auto cpn =
        components[std::stoi(component_nodes[std::stoi(order[c]->id)]->id)];
    std::vector<IRHandle> region;
    for (int i = 0; i < cpn.size(); i++) {
      region.push_back(loop.as<ForNode>()->body[std::stoi(cpn[i]->id)]);
    }
    auto splitedLoop = ForNode::make(VarNode::make(
        IRNodeKeyGen::GetInstance()->YieldVarKey(),
        loop.as<ForNode>()->looping_var_.as<VarNode>()->min,
        loop.as<ForNode>()->looping_var_.as<VarNode>()->max,
        loop.as<ForNode>()->looping_var_.as<VarNode>()->increment));

    SyncParallelHelper helper(loop, splitedLoop);
    for (int i = 0; i < region.size(); i++) {
      std::map<IRNodeKey, IRHandle> dict;

      for (auto var : enclosing_looping_vars) {
        dict[var.as<VarNode>()->id] = var;
      }
      auto clone = region[i].clone(dict);
      helper.visit(clone);

      splitedLoop.as<ForNode>()->Insert(clone);
    }
    counter += region.size();
    // IRMutatorVisitor(loop.as<ForNode>()->looping_var_,
    //                  splitedLoop.as<ForNode>()->looping_var_, false)
    //     .visit(splitedLoop);

    ret.push_back(splitedLoop);
  }
  assert(counter == loop.as<ForNode>()->body.size());
  return ret;
}

void SyncParallel::visitFor(ForHandle loop) {
  enclosing_looping_vars_.push_back(loop->looping_var_);
  for (int i = loop->body.size() - 1; i >= 0; i--) {
    if (loop->body[i].Type() == IRNodeType::FOR) {
      loop->body[i].accept(this);
      // adjust it
      auto adjusted =
          SyncParallel::Adjust(enclosing_looping_vars_, loop->body[i]);
      // std::cout << "erase "
      //           << loop->body[i].as<ForNode>()->looping_var_.as<VarNode>()->id
      //           << "\n";
      loop->body.erase(loop->body.begin() + i);
      for (int j = adjusted.size() - 1; j >= 0; j--) {
        loop->body.insert(loop->body.begin() + i, adjusted[j]);
        // std::cout << "create "
        //           << loop->body[i].as<ForNode>()->looping_var_.as<VarNode>()->id
        //           << "\n";
      }
    }
  }
  enclosing_looping_vars_.pop_back();
}

void SyncParallel::visitFunc(FuncHandle func) {
  for (int i = func->body.size() - 1; i >= 0; i--) {
    if (func->body[i].Type() == IRNodeType::FOR) {
      func->body[i].accept(this);
      // adjust it
      auto adjusted = Adjust(enclosing_looping_vars_, func->body[i]);
      // std::cout << "erase "
      //           << func->body[i].as<ForNode>()->looping_var_.as<VarNode>()->id
      //           << "\n";
      func->body.erase(func->body.begin() + i);
      for (int j = adjusted.size() - 1; j >= 0; j--) {
        func->body.insert(func->body.begin() + i, adjusted[j]);
        // std::cout << "create "
        //           << func->body[i].as<ForNode>()->looping_var_.as<VarNode>()->id
        //           << "\n";
      }
    }
  }
}

}  // namespace polly