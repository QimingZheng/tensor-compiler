#include "sync_parallel.h"

namespace polly {

PassRetHandle SyncParallel::runPass(PassArgHandle arg) {
  SyncParallel sync_par(PassArg::as<SyncParallel::Arg>(arg)->program);
  return SyncParallel::Ret::create();
}

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

  for (int i = 0; i < order.size(); i++) {
    auto cpn =
        components[std::stoi(component_nodes[std::stoi(order[i]->id)]->id)];
    std::vector<IRHandle> region;
    for (int i = 0; i < cpn.size(); i++) {
      region.push_back(loop.as<ForNode>()->body[std::stoi(cpn[i]->id)]);
    }
    auto splitedLoop = ForNode::make(VarNode::make(
        IRNodeKeyGen::GetInstance()->YieldVarKey(),
        loop.as<ForNode>()->looping_var_.as<VarNode>()->min,
        loop.as<ForNode>()->looping_var_.as<VarNode>()->max,
        loop.as<ForNode>()->looping_var_.as<VarNode>()->increment));
    for (int i = 0; i < region.size(); i++) {
      splitedLoop.as<ForNode>()->Insert(region[i]);
    }
    IRMutatorVisitor(loop.as<ForNode>()->looping_var_,
                     splitedLoop.as<ForNode>()->looping_var_, false)
        .visit(splitedLoop);
    ret.push_back(splitedLoop);
  }
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
      loop->body.erase(loop->body.begin() + i);
      for (int j = adjusted.size() - 1; j >= 0; j--) {
        loop->body.insert(loop->body.begin() + i, adjusted[j]);
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
      func->body.erase(func->body.begin() + i);
      for (int j = adjusted.size() - 1; j >= 0; j--) {
        func->body.insert(func->body.begin() + i, adjusted[j]);
      }
    }
  }
}
}  // namespace polly