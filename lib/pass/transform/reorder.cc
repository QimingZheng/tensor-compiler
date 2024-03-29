#include "reorder.h"
#include "pass/analysis/polyhedral_extraction.h"
#include "pass/analysis/polyhedral_model.h"

namespace polly {

class LoopReorderHelper : public IRNotImplementedVisitor {
 public:
  LoopReorderHelper(IRHandle i_loop, IRHandle j_loop)
      : i_loop_(i_loop), j_loop_(j_loop) {}

  IRHandle i_loop_, j_loop_;

  void replace_reordered_loop(DeclHandle decl) {
    auto val = decl->decl.as<ValNode>();
    for (int i = 0; i < val->enclosing_looping_vars_.size(); i++) {
      if (val->enclosing_looping_vars_[i].equals(i_loop_)) {
        val->enclosing_looping_vars_[i] = j_loop_;
      } else if (val->enclosing_looping_vars_[i].equals(j_loop_)) {
        val->enclosing_looping_vars_[i] = i_loop_;
      }
    }
  }

  void visitFor(ForHandle loop) override {
    for (int i = 0; i < loop->body.size(); i++) {
      if (loop->body[i].Type() == IRNodeType::FOR) {
        loop->body[i].accept(this);
      } else if (loop->body[i].Type() == IRNodeType::DECLARATION) {
        replace_reordered_loop(loop->body[i].as<DeclNode>());
      }
    }
  }
  void visitFunc(FuncHandle func) override {
    for (int i = 0; i < func->body.size(); i++) {
      if (func->body[i].Type() == IRNodeType::FOR) {
        func->body[i].accept(this);
      } else if (func->body[i].Type() == IRNodeType::DECLARATION) {
        replace_reordered_loop(func->body[i].as<DeclNode>());
      }
    }
  }
};

LoopReorder::LoopReorder(IRHandle program, IRHandle i_loop, IRHandle j_loop)
    : program_(program), i_loop_(i_loop), j_loop_(j_loop) {
  modeling_ = true;
  ith_ = -1;
  jth_ = -1;
  loop_vars_.clear();

  PolyhedralModel model = PolyhedralExtraction(program_).model;
  std::vector<Iteration> extractedIters;
  std::vector<IRHandle> loop_vars;
  for (int i = 0; i < model.statements_.size(); i++) {
    bool found_i = false, found_j = false;
    std::vector<Iteration> iterations = model.statements_[i].iters_.iterations_;
    for (int j = 0; j < iterations.size(); j++) {
      if (iterations[j].iterName_ == i_loop_.as<VarNode>()->id) {
        found_i = true;
      }
      if (iterations[j].iterName_ == j_loop_.as<VarNode>()->id) {
        found_j = true;
      }
      if (found_i && found_j) {
        extractedIters = std::vector<Iteration>(iterations.begin(),
                                                iterations.begin() + j + 1);
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
}

PassRetHandle LoopReorder::runPass(PassArgHandle arg) {
  LoopReorder(PassArg::as<Arg>(arg)->program, PassArg::as<Arg>(arg)->i_loop,
              PassArg::as<Arg>(arg)->j_loop);
  LoopReorderHelper(PassArg::as<Arg>(arg)->i_loop,
                    PassArg::as<Arg>(arg)->j_loop)
      .visit(PassArg::as<Arg>(arg)->program);
  return Ret::create();
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
    if (ith_ >= 0 && jth_ >= 0) return;
    loop_vars_.pop_back();
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