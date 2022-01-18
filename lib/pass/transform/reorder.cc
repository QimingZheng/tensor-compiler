#include "reorder.h"
#include "pass/analysis/polyhedral_extraction.h"
#include "pass/analysis/polyhedral_model.h"

namespace polly {

PassRetHandle LoopReorder::runPass(PassArgHandle arg) {
  program_ = PassArg::as<Arg>(arg)->program;
  i_loop_ = PassArg::as<Arg>(arg)->i_loop;
  j_loop_ = PassArg::as<Arg>(arg)->j_loop;

  modeling_ = true;
  ith_ = -1;
  ith_ = -1;
  loop_vars_.clear();

  PolyhedralModel model = PolyhedralExtraction(program_).model;
  std::vector<Iteration> extractedIters;
  std::vector<IRHandle> loop_vars;
  for (int i = 0; i < model.statements_.size(); i++) {
    bool found_i = false, found_j;
    std::vector<Iteration> iterations = model.statements_[i].iters_.iterations_;
    for (int j = 0; j < iterations.size(); j++) {
      if (iterations[j].iterName_ == i_loop_.as<VarNode>()->id) {
        found_i = true;
      }
      if (iterations[j].iterName_ == j_loop_.as<VarNode>()->id) {
        found_j = true;
      }
      if (found_i && found_j) {
        extractedIters =
            std::vector<Iteration>(iterations.begin(), iterations.begin() + j);
        break;
      }
    }
    if (found_i && found_j) break;
  }

  program_.accept(this);
  loop_vars = loop_vars_;
  ReorderedBounds rb;
  rb.GetReorderedBound(loop_vars, extractedIters, ith_, jth_);
  modeling_ = false;
  program_.accept(this);
  return std::shared_ptr<Ret>(new Ret);
}

void LoopReorder::visitFor(ForHandle loop) {
  if (!modeling_) {
    for (int i = 0; i < loop->body.size(); i++) {
      loop->body[i].accept(this);
    }

    if (loop->looping_var_.equals(i_loop_)) {
      loop->looping_var_ = j_loop_;
    } else if (loop->looping_var_.equals(j_loop_)) {
      loop->looping_var_ = i_loop_;
    }
  } else {
    if (loop->looping_var_.equals(i_loop_)) {
      ith_ = loop_vars_.size();
    } else if (loop->looping_var_.equals(j_loop_)) {
      jth_ = loop_vars_.size();
    }
    loop_vars_.push_back(loop->looping_var_);

    for (int i = 0; i < loop->body.size(); i++) {
      if (ith_ >= 0 && jth_ >= 0) {
        return;
      }
      loop->body[i].accept(this);
    }
  }
}

void LoopReorder::visitFunc(FuncHandle func) {
  if (!modeling_) {
    for (int i = 0; i < func->body.size(); i++) {
      func->body[i].accept(this);
    }
  } else {
    for (int i = 0; i < func->body.size(); i++) {
      if (ith_ >= 0 && jth_ >= 0) {
        return;
      }
      func->body[i].accept(this);
    }
  }
}

}  // namespace polly