#include "data_dependency_model.h"

namespace polly {

solver::AccessMap DataDependencyModel::BuildAccessMap(solver::context &ctx,
                                                      Statement &st,
                                                      ArrayAccess &acc) {
  std::vector<VarKey> iter_names;
  for (int i = 0; i < st.iters_.iterations_.size(); i++) {
    iter_names.push_back(st.iters_.iterations_[i].iterName_);
  }

  std::vector<std::string> indices_names;
  for (int i = 0; i < acc.access.indices_.size(); i++) {
    indices_names.push_back("x" + std::to_string(i));
  }

  solver::AccessMap ret = solver::AccessMap(
      ctx, st.statementName, iter_names, acc.access.arrayName_, indices_names);

  for (int i = 0; i < acc.access.indices_.size(); i++) {
    std::map<std::string, int> ind = acc.access.indices_[i].coeffs;
    ind["x" + std::to_string(i)] = -1;
    ret.add_constraint(
        ret.CreateEquality(ind, acc.access.indices_[i].constant));
  }

  for (int i = 0; i < st.iters_.iterations_.size(); i++) {
    Iteration iter = st.iters_.iterations_[i];
    for (auto lb : iter.lowerBounds_) {
      std::map<std::string, int> bound = lb.coeffs;
      for (auto &i : bound) {
        i.second = -i.second;
      }
      bound[iter.iterName_] = 1;
      ret.add_constraint(ret.CreateInequality(bound, -lb.constant));
    }
  }

  for (int i = 0; i < st.iters_.iterations_.size(); i++) {
    Iteration iter = st.iters_.iterations_[i];
    for (auto ub : iter.upperBounds_) {
      std::map<std::string, int> bound = ub.coeffs;
      bound[iter.iterName_] = -1;
      ret.add_constraint(ret.CreateInequality(bound, ub.constant));
    }
  }
  return ret;
}

DataDependencyModel::DataDependencyModel(solver::context &ctx,
                                         PolyhedralModel model)
    : model(model) {
  int deepest = 0;

  std::map<StatementKey, Statement> statements;

  for (Statement st : model.statements_) {
    IterDomain iter = st.iters_;
    ProgDomain prog = st.prog_;
    assert(iter.iterations_.size() + 1 == prog.progContext_.size());
    deepest = std::max(
        deepest, (int)(iter.iterations_.size() + prog.progContext_.size()));
    statements[st.statementName] = st;
  }

  depth = deepest;

  std::vector<solver::AccessMap> reads;
  std::vector<std::vector<std::string>> readsIters;
  std::vector<std::vector<int>> readsProgs;
  std::vector<solver::AccessMap> writes;
  std::vector<std::vector<std::string>> writesIters;
  std::vector<std::vector<int>> writesProgs;

  for (Statement st : model.statements_) {
    auto accesses = st.accesses_;
    for (ArrayAccess acc : accesses) {
      if (acc.access.type_ == ArrayDomain::AccessType::WRITE) {
        writes.push_back(BuildAccessMap(ctx, st, acc));
        writesIters.push_back(st.iters_.GerIters());
        writesProgs.push_back(st.prog_.progContext_);
      } else {
        reads.push_back(BuildAccessMap(ctx, st, acc));
        readsIters.push_back(st.iters_.GerIters());
        readsProgs.push_back(st.prog_.progContext_);
      }
    }
  }

  RAW = solver::DependencyMap(ctx, reads, writes, readsIters, writesIters,
                              readsProgs, writesProgs, deepest);
  WAR = solver::DependencyMap(ctx, writes, reads, writesIters, readsIters,
                              writesProgs, readsProgs, deepest);
  WAW = solver::DependencyMap(ctx, writes, writes, writesIters, writesIters,
                              writesProgs, writesProgs, deepest);
}

DataDependencyModel::DataDependencyModel(solver::context &ctx,
                                         std::vector<int> prog_context,
                                         PolyhedralModel model)
    : model(model) {
  int deepest = 0;

  std::map<StatementKey, Statement> statements;

  for (Statement st : model.statements_) {
    IterDomain iter = st.iters_;
    ProgDomain prog = st.prog_;
    assert(iter.iterations_.size() + 1 == prog.progContext_.size());
    deepest = std::max(
        deepest, (int)(iter.iterations_.size() + prog.progContext_.size()));
    statements[st.statementName] = st;
  }

  depth = deepest;

  std::vector<solver::AccessMap> reads;
  std::vector<std::vector<std::string>> readsIters;
  std::vector<std::vector<int>> readsProgs;
  std::vector<solver::AccessMap> writes;
  std::vector<std::vector<std::string>> writesIters;
  std::vector<std::vector<int>> writesProgs;

  for (Statement st : model.statements_) {
    auto accesses = st.accesses_;
    for (ArrayAccess acc : accesses) {
      if (acc.access.type_ == ArrayDomain::AccessType::WRITE) {
        writes.push_back(BuildAccessMap(ctx, st, acc));
        writesIters.push_back(st.iters_.GerIters());
        writesProgs.push_back(st.prog_.progContext_);
      } else {
        reads.push_back(BuildAccessMap(ctx, st, acc));
        readsIters.push_back(st.iters_.GerIters());
        readsProgs.push_back(st.prog_.progContext_);
      }
    }
  }

  RAW = solver::DependencyMap(ctx, reads, writes, readsIters, writesIters,
                              readsProgs, writesProgs, deepest);
  WAR = solver::DependencyMap(ctx, writes, reads, writesIters, readsIters,
                              writesProgs, readsProgs, deepest);
  WAW = solver::DependencyMap(ctx, writes, writes, writesIters, writesIters,
                              writesProgs, writesProgs, deepest);

  RAW = solver::DependencyMap(ctx, prog_context, deepest, RAW.dependency);
  WAR = solver::DependencyMap(ctx, prog_context, deepest, WAR.dependency);
  WAW = solver::DependencyMap(ctx, prog_context, deepest, WAW.dependency);
}

solver::union_map DataDependencyModel::CreateTransformMap(
    solver::context &ctx, PolyhedralModel origin, PolyhedralModel transformed) {
  std::map<StatementKey, Statement> originStatements;
  std::map<StatementKey, Statement> transformedStatements;

  int deepest = 0;

  for (int i = 0; i < origin.statements_.size(); i++) {
    originStatements[origin.statements_[i].statementName] =
        origin.statements_[i];
    deepest = std::max(deepest,
                       (int)(origin.statements_[i].iters_.iterations_.size() +
                             origin.statements_[i].prog_.progContext_.size()));
  }

  for (int i = 0; i < transformed.statements_.size(); i++) {
    transformedStatements[transformed.statements_[i].statementName] =
        transformed.statements_[i];
  }
  solver::union_map ret(ctx, "{}");

  for (auto it : originStatements) {
    auto ori_st = it.second;
    auto tr_st = transformedStatements[it.first];
    ret = ret | solver::ScheduleMap::OneOneMap(
                    ctx, ori_st.iters_.GerIters(), ori_st.prog_.progContext_,
                    tr_st.iters_.GerIters(), tr_st.prog_.progContext_, deepest);
  }
  return ret;
}

}  // namespace polly