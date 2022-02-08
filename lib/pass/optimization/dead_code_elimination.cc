#include "dead_code_elimination.h"

namespace polly {

void DeadCodeElimination::visitFor(ForHandle loop) {
  for (int i = 0; i < loop->body.size(); i++) {
    if (loop->body[i].Type() == IRNodeType::FOR) {
      enclosing_looping_vars_.push_back(
          loop->body[i].as<ForNode>()->looping_var_);

      loop->body[i].accept(this);

      if (loop->body[i].as<ForNode>()->body.size() == 0 ||
          EmptyBounds::IsEmptyPolyhedral(
              enclosing_looping_vars_,
              PolyhedralExtraction::ExtractIterDomain(enclosing_looping_vars_)
                  .iterations_)) {
        loop->body.erase(loop->body.begin() + i);
        i -= 1;
      }

      enclosing_looping_vars_.pop_back();
    }
  }
}

void DeadCodeElimination::visitFunc(FuncHandle func) {
  for (int i = 0; i < func->body.size(); i++) {
    if (func->body[i].Type() == IRNodeType::FOR) {
      enclosing_looping_vars_.push_back(
          func->body[i].as<ForNode>()->looping_var_);

      func->body[i].accept(this);

      if (func->body[i].as<ForNode>()->body.size() == 0 ||
          EmptyBounds::IsEmptyPolyhedral(
              enclosing_looping_vars_,
              PolyhedralExtraction::ExtractIterDomain(enclosing_looping_vars_)
                  .iterations_)) {
        func->body.erase(func->body.begin() + i);
        i -= 1;
      }

      enclosing_looping_vars_.pop_back();
    }
  }
}

}  // namespace polly