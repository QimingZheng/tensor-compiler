#include "fussion.h"

namespace polly {

PassRetHandle FussionTransform::runPass(PassArgHandle arg) {
  FussionTransform(PassArg::as<Arg>(arg)->program,
                   PassArg::as<Arg>(arg)->firstLoop,
                   PassArg::as<Arg>(arg)->secondLoop);
  return Ret::create();
}

IRHandle FussionTransform::replace_if_match(IRHandle origin) {
  origin.accept(this);
  if (origin.equals(secondLoop.as<ForNode>()->looping_var_)) {
    return firstLoop.as<ForNode>()->looping_var_;
  } else {
    return origin;
  }
}

bool FussionTransform::findLoop(std::vector<IRHandle>& handles,
                                IRHandle target) {
  for (int i = 0; i < handles.size(); i++) {
    if (handles[i].equals(target)) {
      return true;
    }
  }
  return false;
}

void FussionTransform::visitInt(IntHandle int_expr) {
  /// Pass
}

void FussionTransform::visitAdd(AddHandle add) {
  if (!searching_) {
    add->lhs = replace_if_match(add->lhs);
    add->rhs = replace_if_match(add->rhs);
  }
}

void FussionTransform::visitSub(SubHandle sub) {
  if (!searching_) {
    sub->lhs = replace_if_match(sub->lhs);
    sub->rhs = replace_if_match(sub->rhs);
  }
}

void FussionTransform::visitMul(MulHandle mul) {
  if (!searching_) {
    mul->lhs = replace_if_match(mul->lhs);
    mul->rhs = replace_if_match(mul->rhs);
  }
}

void FussionTransform::visitDiv(DivHandle div) {
  if (!searching_) {
    div->lhs = replace_if_match(div->lhs);
    div->rhs = replace_if_match(div->rhs);
  }
}

void FussionTransform::visitMod(ModHandle mod) {
  if (!searching_) {
    mod->lhs = replace_if_match(mod->lhs);
    mod->rhs = replace_if_match(mod->rhs);
  }
}

void FussionTransform::visitVar(VarHandle var) {
  if (!searching_) {
    var->min = replace_if_match(var->min);
    var->max = replace_if_match(var->max);
    var->increment = replace_if_match(var->increment);
  }
}

void FussionTransform::visitAccess(AccessHandle access) {
  if (!searching_) {
    for (int i = 0; i < access->indices.size(); i++) {
      access->indices[i] = replace_if_match(access->indices[i]);
    }
  }
}

void FussionTransform::visitAssign(AssignmentHandle assign) {
  if (!searching_) {
    assign->lhs = replace_if_match(assign->lhs);
    assign->rhs = replace_if_match(assign->rhs);
  }
}

void FussionTransform::visitTensor(TensorHandle tensor) {
  /// Pass
}

void FussionTransform::visitFor(ForHandle loop) {
  if (searching_) {
    if (findLoop(loop->body, firstLoop) && findLoop(loop->body, secondLoop)) {
      searching_ = false;
      secondLoop.accept(this);
      for (int i = 0; i < secondLoop.as<ForNode>()->body.size(); i++) {
        firstLoop.as<ForNode>()->Insert(secondLoop.as<ForNode>()->body[i]);
      }
      for (int i = 0; i < loop->body.size(); i++) {
        if (loop->body[i].equals(secondLoop)) {
          loop->body.erase(loop->body.begin() + i);
          break;
        }
      }
    }
  } else {
    loop->looping_var_.accept(this);
    for (int i = 0; i < loop->body.size(); i++) {
      loop->body[i].accept(this);
    }
  }
}

void FussionTransform::visitConst(ConstHandle con) {
  /// Pass
}

void FussionTransform::visitPrint(PrintHandle print) {
  print->print = replace_if_match(print->print);
  print->print.accept(this);
}

void FussionTransform::visitFunc(FuncHandle func) {
  if (searching_) {
    if (findLoop(func->body, firstLoop) && findLoop(func->body, secondLoop)) {
      searching_ = false;
      secondLoop.accept(this);
      for (int i = 0; i < secondLoop.as<ForNode>()->body.size(); i++) {
        firstLoop.as<ForNode>()->Insert(secondLoop.as<ForNode>()->body[i]);
      }
      for (int i = 0; i < func->body.size(); i++) {
        if (func->body[i].equals(secondLoop)) {
          func->body.erase(func->body.begin() + i);
          break;
        }
      }
    }
  } else {
    for (int i = 0; i < func->body.size(); i++) {
      func->body[i].accept(this);
    }
  }
}

}  // namespace polly