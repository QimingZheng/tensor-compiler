#include "loop_parallel.h"

namespace polly {

void LoopParallel::visitFor(ForHandle loop) {
  auto ret = ParallelizationAnalysisPass::runPass(
      ParallelizationAnalysisPass::Arg::create(program_, IRHandle(loop)));
  loop->annotation.parallelization =
      PassRet::as<ParallelizationAnalysisPass::Ret>(ret)->legal;

  for (int i = 0; i < loop->body.size(); i++) {
    if (loop->body[i].Type() == IRNodeType::FOR) {
      loop->body[i].accept(this);
    }
  }
}

void LoopParallel::visitFunc(FuncHandle func) {
  for (int i = 0; i < func->body.size(); i++) {
    if (func->body[i].Type() == IRNodeType::FOR) {
      func->body[i].accept(this);
    }
  }
}

}  // namespace polly