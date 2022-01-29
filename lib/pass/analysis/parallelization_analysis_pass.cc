#include "parallelization_analysis_pass.h"

namespace polly {

PassRetHandle ParallelizationAnalysisPass::runPass(PassArgHandle arg) {
  auto program = PassArg::as<Arg>(arg)->program;
  auto loop = PassArg::as<Arg>(arg)->loop;
  ParallelizationAnalysisPass analysis(program, loop);

  auto model = PolyhedralExtraction(program).model;

  solver::context ctx;
  auto dependency = DataDependencyModel(ctx, model);

  std::vector<std::string> iters;
  for (int i = 0; i < analysis.nesting_loop_vars_.size(); i++) {
    iters.push_back(analysis.nesting_loop_vars_[i]
                        .as<ForNode>()
                        ->looping_var_.as<VarNode>()
                        ->id);
  }

  int deepest = 0;

  for (Statement st : model.statements_) {
    IterDomain iter = st.iters_;
    ProgDomain prog = st.prog_;
    deepest = std::max(
        deepest, (int)(iter.iterations_.size() + prog.progContext_.size()));
  }

  auto parMap = solver::ScheduleMap::ParallelMap(
      ctx, iters, analysis.prog_context_, deepest);

  return Ret::create(!((!(dependency.RAW.dependency & parMap).empty()) ||
                       (!(dependency.WAW.dependency & parMap).empty()) ||
                       (!(dependency.WAR.dependency & parMap).empty())));
}

void ParallelizationAnalysisPass::visitFor(ForHandle loop) {
  nesting_loop_vars_.push_back(IRHandle(loop));
  if (loop->looping_var_.equals(loop_.as<ForNode>()->looping_var_)) {
    found_ = true;
    return;
  } else {
    int i = 0;
    for (auto it : loop->body) {
      prog_context_.push_back(i);
      if (it.Type() == IRNodeType::FOR) {
        it.accept(this);
      }
      if (found_) return;
      prog_context_.pop_back();
      i += 1;
    }
  }
  nesting_loop_vars_.pop_back();
}

void ParallelizationAnalysisPass::visitFunc(FuncHandle func) {
  int i = 0;
  for (auto it : func->body) {
    prog_context_.push_back(i);
    if (it.Type() == IRNodeType::FOR) {
      it.accept(this);
    }
    if (found_) return;
    prog_context_.pop_back();
    i += 1;
  }
}

}  // namespace polly