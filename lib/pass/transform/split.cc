/*
 * Created Date: 17th January 2022
 * Author: Qiming Zheng
 * Copyright (c) 2022 Qiming Zheng
 */

#include "split.h"

namespace polly {

PassRetHandle LoopSplit::runPass(PassArgHandle arg) {
  searching_ = true;
  program_ = PassArg::as<Arg>(arg)->program;
  loop_ = PassArg::as<Arg>(arg)->loop;
  splitFactor = PassArg::as<Arg>(arg)->splitFactor;
  this->loop_.accept(this);
  auto ret = std::shared_ptr<Ret>(new Ret);
  return ret;
}

IRHandle LoopSplit::replace_with(IRHandle node) {
  if (node.equals(loop_.as<ForNode>()->looping_var_)) {
    return replace_;
  }
  node.accept(this);
  return node;
}

void LoopSplit::visitInt(IntHandle int_expr) {
  /// Pass
}

void LoopSplit::visitAdd(AddHandle add) {
  if (!searching_) {
    add->lhs = replace_with(add->lhs);
    add->rhs = replace_with(add->rhs);
  }
}

void LoopSplit::visitSub(SubHandle sub) {
  if (!searching_) {
    sub->lhs = replace_with(sub->lhs);
    sub->rhs = replace_with(sub->rhs);
  }
}

void LoopSplit::visitMul(MulHandle mul) {
  if (!searching_) {
    mul->lhs = replace_with(mul->lhs);
    mul->rhs = replace_with(mul->rhs);
  }
}

void LoopSplit::visitDiv(DivHandle div) {
  if (!searching_) {
    div->lhs = replace_with(div->lhs);
    div->rhs = replace_with(div->rhs);
  }
}

void LoopSplit::visitMod(ModHandle mod) {
  if (!searching_) {
    mod->lhs = replace_with(mod->lhs);
    mod->rhs = replace_with(mod->rhs);
  }
}

void LoopSplit::visitVar(VarHandle var) {
  if (!searching_) {
    var->min = replace_with(var->min);
    var->max = replace_with(var->max);
    var->increment = replace_with(var->increment);
  }
}

void LoopSplit::visitAccess(AccessHandle access) {
  if (!searching_) {
    for (int i = 0; i < access->indices.size(); i++) {
      access->indices[i] = replace_with(access->indices[i]);
    }
  }
}

void LoopSplit::visitAssign(AssignmentHandle assign) {
  if (!searching_) {
    assign->lhs = replace_with(assign->lhs);
    assign->rhs = replace_with(assign->rhs);
  }
}

void LoopSplit::visitTensor(TensorHandle tensor) {
  /// Pass
}

void LoopSplit::visitFor(ForHandle loop) {
  if (searching_) {
    for (int i = 0; i < loop->body.size(); i++) {
      if (loop->body[i].equals(loop_)) {
        searching_ = false;
        auto ori_loop_var = loop_.as<ForNode>()->looping_var_;
        auto outter_loop_var = VarNode::make(
            "i" + IRNodeKeyGen::GetInstance()->yield(), IntNode::make(0),
            DivNode::make(
                SubNode::make(
                    AddNode::make(ori_loop_var.as<VarNode>()->max,
                                  SubNode::make(IntNode::make(splitFactor),
                                                IntNode::make(1))),
                    ori_loop_var.as<VarNode>()->min),
                IntNode::make(splitFactor)),
            IntNode::make(1));
        auto outter_loop = ForNode::make(outter_loop_var);

        auto inner_loop_var = VarNode::make(
            "i" + IRNodeKeyGen::GetInstance()->yield(), IntNode::make(0),
            IntNode::make(splitFactor), IntNode::make(1));
        auto inner_loop = ForNode::make(inner_loop_var);

        outter_loop.as<ForNode>()->Insert(inner_loop);

        replace_ = AddNode::make(
            MulNode::make(outter_loop_var, IntNode::make(splitFactor)),
            inner_loop_var);

        loop->body[i].accept(this);

        for (int j = 0; j < loop->body[i].as<ForNode>()->body.size(); j++) {
          inner_loop.as<ForNode>()->Insert(
              loop->body[i].as<ForNode>()->body[j]);
        }

        loop->body[i] = outter_loop;

        break;
      }
    }
  }

  if (!searching_) {
    for (int i = 0; i < loop->body.size(); i++) {
      loop->body[i].accept(this);
    }
  }
}

void LoopSplit::visitConst(ConstHandle con) {
  throw std::runtime_error("Should not visit a constant node");
}

void LoopSplit::visitPrint(PrintHandle print) {
  if (!searching_) print->print = replace_with(print->print);
}

void LoopSplit::visitFunc(FuncHandle func) {
  for (int i = 0; i < func->body.size(); i++) {
    if (func->body[i].equals(loop_)) {
      searching_ = false;
      auto ori_loop_var = loop_.as<ForNode>()->looping_var_;
      auto outter_loop_var = VarNode::make(
          "i" + IRNodeKeyGen::GetInstance()->yield(), IntNode::make(0),
          DivNode::make(
              SubNode::make(
                  AddNode::make(ori_loop_var.as<VarNode>()->max,
                                SubNode::make(IntNode::make(splitFactor),
                                              IntNode::make(1))),
                  ori_loop_var.as<VarNode>()->min),
              IntNode::make(splitFactor)),
          IntNode::make(1));
      auto outter_loop = ForNode::make(outter_loop_var);

      auto inner_loop_var = VarNode::make(
          "i" + IRNodeKeyGen::GetInstance()->yield(), IntNode::make(0),
          IntNode::make(splitFactor), IntNode::make(1));
      auto inner_loop = ForNode::make(inner_loop_var);

      outter_loop.as<ForNode>()->Insert(inner_loop);

      replace_ = AddNode::make(
          MulNode::make(outter_loop_var, IntNode::make(splitFactor)),
          inner_loop_var);

      func->body[i].accept(this);

      for (int j = 0; j < func->body[i].as<ForNode>()->body.size(); j++) {
        inner_loop.as<ForNode>()->Insert(func->body[i].as<ForNode>()->body[j]);
      }

      func->body[i] = outter_loop;

      break;
    }
  }
  if (searching_) {
    for (int i = 0; i < func->body.size(); i++) {
      func->body[i].accept(this);
    }
  }
}

}  // namespace polly