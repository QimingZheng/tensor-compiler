#include "fission.h"

namespace polly {

void FissionTransform::Transform() { module_.GetRoot().accept(this); }

void FissionTransform::visitInt(IntHandle int_expr) {
  /// Pass
}

IRHandle FissionTransform::replace_if_match(IRHandle origin) {
  if (origin.equals(loop_)) {
    return replace_loop_;
  } else {
    return origin;
  }
}

void FissionTransform::visitAdd(AddHandle add) {
  if (!searching_) {
    add->lhs = replace_if_match(add->lhs);
    add->rhs = replace_if_match(add->rhs);
  }
}

void FissionTransform::visitSub(SubHandle sub) {
  if (!searching_) {
    sub->lhs = replace_if_match(sub->lhs);
    sub->rhs = replace_if_match(sub->rhs);
  }
}

void FissionTransform::visitMul(MulHandle mul) {
  if (!searching_) {
    mul->lhs = replace_if_match(mul->lhs);
    mul->rhs = replace_if_match(mul->rhs);
  }
}

void FissionTransform::visitDiv(DivHandle div) {
  if (!searching_) {
    div->lhs = replace_if_match(div->lhs);
    div->rhs = replace_if_match(div->rhs);
  }
}

void FissionTransform::visitMod(ModHandle mod) {
  if (!searching_) {
    mod->lhs = replace_if_match(mod->lhs);
    mod->rhs = replace_if_match(mod->rhs);
  }
}

void FissionTransform::visitVar(VarHandle var) {
  if (!searching_) {
    var->min = replace_if_match(var->min);
    var->max = replace_if_match(var->max);
    var->increment = replace_if_match(var->increment);
  }
}

void FissionTransform::visitAccess(AccessHandle access) {
  if (!searching_) {
    for (int i = 0; i < access->indices.size(); i++) {
      access->indices[i] = replace_if_match(access->indices[i]);
    }
  }
}

void FissionTransform::visitAssign(AssignmentHandle assign) {
  if (!searching_) {
    assign->lhs = replace_if_match(assign->lhs);
    assign->rhs = replace_if_match(assign->rhs);
  }
}

void FissionTransform::visitTensor(TensorHandle tensor) {
  /// Pass
}

void FissionTransform::visitFor(ForHandle loop) {
  if (searching_) {
    for (int i = 0; i < loop->body.size(); i++) {
      if (loop->body[i].equals(loop_)) {
        searching_ = true;
        auto targetLoop = loop->body[i].as<ForNode>();
        std::vector<IRHandle> fissionLoops;
        for (int j = 0; j < targetLoop->body.size(); j++) {
          fissionLoops.push_back(ForNode::make(VarNode::make(
              targetLoop->looping_var_.as<VarNode>()->name,
              targetLoop->looping_var_.as<VarNode>()->min,
              targetLoop->looping_var_.as<VarNode>()->max,
              targetLoop->looping_var_.as<VarNode>()->increment)));
        }
        for (int j = 0; j < targetLoop->body.size(); j++) {
          replace_loop_ = fissionLoops[j];
          targetLoop->body[j].accept(this);
          fissionLoops[j].as<ForNode>()->Insert(targetLoop->body[j]);
        }
        loop->body.erase(loop->body.begin() + i);
        for (int j = 0; j < fissionLoops.size(); j++) {
          loop->body.insert(loop->body.begin() + i + j, fissionLoops[j]);
        }
        break;
      }
    }
    if (!searching_) {
      for (int i = 0; i < loop->body.size(); i++) {
        loop->body[i].accept(this);
      }
    }
  } else {
    loop->looping_var_.accept(this);
    for (int i = 0; i < loop->body.size(); i++) {
      loop->body[i].accept(this);
    }
  }
}

void FissionTransform::visitConst(ConstHandle con) {
  /// Pass
}

void FissionTransform::visitPrint(PrintHandle print) {
  if (!searching_) {
    if (print->print.equals(replace_loop_)) {
      print->print = replace_loop_;
    } else {
      print->print.accept(this);
    }
  }
}

void FissionTransform::visitFunc(FuncHandle func) {
  for (int i = 0; i < func->body.size(); i++) {
    if (func->body[i].equals(loop_)) {
      searching_ = true;
      auto loop = func->body[i].as<ForNode>();
      std::vector<IRHandle> fissionLoops;
      for (int j = 0; j < loop->body.size(); j++) {
        fissionLoops.push_back(ForNode::make(
            VarNode::make(loop->looping_var_.as<VarNode>()->name,
                          loop->looping_var_.as<VarNode>()->min,
                          loop->looping_var_.as<VarNode>()->max,
                          loop->looping_var_.as<VarNode>()->increment)));
      }
      for (int j = 0; j < loop->body.size(); j++) {
        replace_loop_ = fissionLoops[j];
        loop->body[j].accept(this);
        fissionLoops[j].as<ForNode>()->Insert(loop->body[j]);
      }
      func->body.erase(func->body.begin() + i);
      for (int j = 0; j < fissionLoops.size(); j++) {
        func->body.insert(func->body.begin() + i + j, fissionLoops[j]);
      }
      break;
    }
  }
  if (!searching_) {
    for (int i = 0; i < func->body.size(); i++) {
      func->body[i].accept(this);
    }
  }
}

}  // namespace polly