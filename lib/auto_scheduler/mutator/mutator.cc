#include "mutator.h"

namespace polly {

bool Mutator::OfSameScope(IRHandle program, IRHandle first_loop,
                          IRHandle second_loop) {
  // two loops cannot be the same.
  if (first_loop.equals(second_loop)) return false;

  bool found_first = false;
  bool found_second = false;

  std::queue<IRHandle> q;
  for (auto it : program.as<FuncNode>()->body) {
    q.push(it);
    if (it.equals(first_loop)) found_first = true;
    if (it.equals(second_loop)) found_second = true;
  }
  if (found_first && found_second) return true;
  if (found_first && !found_second) return false;
  if (!found_first && found_second) return false;

  while (!q.empty()) {
    auto cur = q.front();
    q.pop();
    if (cur.Type() == IRNodeType::FOR) {
      auto loop = cur.as<ForNode>();
      for (int i = 0; i < loop->body.size(); i++) {
        q.push(loop->body[i]);
        if (loop->body[i].equals(first_loop)) found_first = true;
        if (loop->body[i].equals(second_loop)) found_second = true;
      }
    }
    if (found_first && found_second) return true;
    if (found_first && !found_second) return false;
    if (!found_first && found_second) return false;
  }
  return false;
}

bool Mutator::IsFullyNested(IRHandle outter_loop, IRHandle inner_loop) {
  // two loops cannot be the same.
  if (outter_loop.equals(inner_loop)) return false;

  std::queue<IRHandle> q;
  q.push(outter_loop);
  while (!q.empty()) {
    auto cur = q.front();
    q.pop();
    if (cur.equals(inner_loop)) return true;
    if (cur.Type() != IRNodeType::FOR) {
      return false;
    } else {
      auto loop = cur.as<ForNode>();
      for (int i = 0; i < loop->body.size(); i++) {
        q.push(loop->body[i]);
      }
      if (loop->body.size() > 0) {
        return false;
      }
    }
  }
  return false;
}

bool Mutator::Split(IRHandle program, IRHandle loop, int splitFactor) {
  if (splitFactor <= 0) return false;
  NormalizationPass::runPass(NormalizationPass::Arg::create(program));
  // A loop in a program can be split iff the program is normalized.
  auto looping_var = loop.as<ForNode>()->looping_var_.as<VarNode>();
  if (!looping_var->min.equals(IntNode::make(0))) {
    return false;
  }
  if (!looping_var->increment.equals(IntNode::make(1))) {
    return false;
  }
  LoopSplit::runPass(LoopSplit::Arg::create(program, loop, splitFactor));
  return true;
}

bool Mutator::Reorder(IRHandle program, IRHandle outter_loop,
                      IRHandle inner_loop) {
  NormalizationPass::runPass(NormalizationPass::Arg::create(program));
  // Two loops can be reordered only when
  // 1. Both are in the same consecutive nestings.
  if (!Mutator::IsFullyNested(outter_loop, inner_loop)) {
    return false;
  }
  std::map<IRNodeKey, IRHandle> dict;
  auto oriProg = program.clone(dict);

  LoopReorder::runPass(
      LoopReorder::Arg::create(program, outter_loop, inner_loop));

  auto ret = ReorderTransformAnalysisPass::runPass(
      ReorderTransformAnalysisPass::Arg::create(oriProg, program));

  return PassRet::as<ReorderTransformAnalysisPass::Ret>(ret)->legal;
}

bool Mutator::Fussion(IRHandle program, IRHandle first_loop,
                      IRHandle second_loop) {
  NormalizationPass::runPass(NormalizationPass::Arg::create(program));
  auto first_looping_var = first_loop.as<ForNode>()->looping_var_.as<VarNode>();
  auto second_looping_var =
      second_loop.as<ForNode>()->looping_var_.as<VarNode>();
  if (!first_looping_var->min.equals(second_looping_var->min)) {
    return false;
  }
  if (!first_looping_var->max.equals(second_looping_var->max)) {
    return false;
  }
  if (!first_looping_var->increment.equals(second_looping_var->increment)) {
    return false;
  }
  /// Check the first_loop and the second_loop are in the same scope.
  if (!OfSameScope(program, first_loop, second_loop)) return false;

  std::map<IRNodeKey, IRHandle> dict;
  auto oriProg = program.clone(dict);

  FussionTransform::runPass(
      FussionTransform::Arg::create(program, first_loop, second_loop));

  auto ret = FussionTransformAnalysisPass::runPass(
      FussionTransformAnalysisPass::Arg::create(oriProg, program));

  return PassRet::as<FussionTransformAnalysisPass::Ret>(ret)->legal;
}

bool Mutator::Fission(IRHandle program, IRHandle loop) {
  NormalizationPass::runPass(NormalizationPass::Arg::create(program));

  std::map<IRNodeKey, IRHandle> dict;
  auto oriProg = program.clone(dict);

  FissionTransform::runPass(FissionTransform::Arg::create(program, loop));

  auto ret = FissionTransformAnalysisPass::runPass(
      FissionTransformAnalysisPass::Arg::create(oriProg, program));

  bool res = PassRet::as<FissionTransformAnalysisPass::Ret>(ret)->legal;
  return res;
}

bool Mutator::Unroll(IRHandle program) {
  NormalizationPass::runPass(NormalizationPass::Arg::create(program));

  LoopUnroll::runPass(LoopUnroll::Arg::create(program));

  return true;
}

}  // namespace polly