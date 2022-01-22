/*
 * Created Date: 17th January 2022
 * Author: Qiming Zheng
 * Copyright (c) 2022 Qiming Zheng
 */

#include "split.h"

namespace polly {

PassRetHandle LoopSplit::runPass(PassArgHandle arg) {
  LoopSplit split(PassArg::as<Arg>(arg)->program, PassArg::as<Arg>(arg)->loop,
                  PassArg::as<Arg>(arg)->splitFactor);
  return Ret::create();
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
        auto outter_loop_var = get_outter_loop_var(ori_loop_var);
        auto outter_loop = ForNode::make(outter_loop_var);
        auto inner_loop_var = get_inner_loop_var();
        auto inner_loop = ForNode::make(inner_loop_var);

        outter_loop.as<ForNode>()->Insert(inner_loop);

        auto remainder_loop_var = get_remainder_loop_var(ori_loop_var);
        auto remainder_loop =
            create_remainder_loop(loop->body[i], remainder_loop_var);

        loop_ = loop->body[i];
        replace_ = AddNode::make(
            MulNode::make(outter_loop_var, IntNode::make(splitFactor)),
            inner_loop_var);

        loop->body[i].accept(this);

        for (int j = 0; j < loop->body[i].as<ForNode>()->body.size(); j++) {
          inner_loop.as<ForNode>()->Insert(
              loop->body[i].as<ForNode>()->body[j]);
        }

        loop->body[i] = outter_loop;
        loop->body.insert(loop->body.begin() + i + 1, remainder_loop);

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
      auto outter_loop_var = get_outter_loop_var(ori_loop_var);
      auto outter_loop = ForNode::make(outter_loop_var);
      auto inner_loop_var = get_inner_loop_var();
      auto inner_loop = ForNode::make(inner_loop_var);
      outter_loop.as<ForNode>()->Insert(inner_loop);

      auto remainder_loop_var = get_remainder_loop_var(ori_loop_var);
      auto remainder_loop =
          create_remainder_loop(func->body[i], remainder_loop_var);

      // amend_remainder_loop(remainder_loop, remainder_loop_var);

      loop_ = func->body[i];
      replace_ = AddNode::make(
          MulNode::make(outter_loop_var, IntNode::make(splitFactor)),
          inner_loop_var);
      func->body[i].accept(this);
      for (int j = 0; j < func->body[i].as<ForNode>()->body.size(); j++) {
        inner_loop.as<ForNode>()->Insert(func->body[i].as<ForNode>()->body[j]);
      }
      func->body[i] = outter_loop;
      func->body.insert(func->body.begin() + i + 1, remainder_loop);

      break;
    }
  }
  if (searching_) {
    for (int i = 0; i < func->body.size(); i++) {
      func->body[i].accept(this);
    }
  }
}

IRHandle LoopSplit::get_outter_loop_var(IRHandle loop_var) {
  return VarNode::make(
      "i" + IRNodeKeyGen::GetInstance()->yield(), IntNode::make(0),
      DivNode::make(loop_var.as<VarNode>()->max, IntNode::make(splitFactor)),
      IntNode::make(1));
}

IRHandle LoopSplit::get_inner_loop_var() {
  return VarNode::make("i" + IRNodeKeyGen::GetInstance()->yield(),
                       IntNode::make(0), IntNode::make(splitFactor),
                       IntNode::make(1));
}

IRHandle LoopSplit::get_remainder_loop_var(IRHandle loop_var) {
  return VarNode::make("i" + IRNodeKeyGen::GetInstance()->yield(),
                       MulNode::make(IntNode::make(splitFactor),
                                     DivNode::make(loop_var.as<VarNode>()->max,
                                                   IntNode::make(splitFactor))),
                       loop_var.as<VarNode>()->max, IntNode::make(1));
}

class LoopSplitHelper : public IRNotImplementedVisitor {
 public:
  std::map<IRNodeKey, IRHandle> dict;

  IRHandle node;
  void visitInt(IntHandle int_expr) override {
    node = IntNode::make(int_expr->value);
  }
  void visitAdd(AddHandle add) override {
    add->lhs.accept(this);
    auto lhs = node;
    add->rhs.accept(this);
    auto rhs = node;
    node = AddNode::make(lhs, rhs);
  }
  void visitSub(SubHandle sub) override {
    sub->lhs.accept(this);
    auto lhs = node;
    sub->rhs.accept(this);
    auto rhs = node;
    node = SubNode::make(lhs, rhs);
  }
  void visitMul(MulHandle mul) override {
    mul->lhs.accept(this);
    auto lhs = node;
    mul->rhs.accept(this);
    auto rhs = node;
    node = MulNode::make(lhs, rhs);
  }
  void visitDiv(DivHandle div) override {
    div->lhs.accept(this);
    auto lhs = node;
    div->rhs.accept(this);
    auto rhs = node;
    node = DivNode::make(lhs, rhs);
  }
  void visitMod(ModHandle mod) override {
    mod->lhs.accept(this);
    auto lhs = node;
    mod->rhs.accept(this);
    auto rhs = node;
    node = ModNode::make(lhs, rhs);
  }
  void visitVar(VarHandle var) override {
    if (dict.find(var->id) != dict.end()) {
      node = dict[var->id];
    } else {
      node = IRHandle(var);
    }
  }
  void visitAccess(AccessHandle access) override {
    access->tensor.accept(this);
    auto tensor = node;
    std::vector<IRHandle> indices;
    for (int i = 0; i < access->indices.size(); i++) {
      access->indices[i].accept(this);
      indices.push_back(node);
    }
    node = AccessNode::make(tensor, indices);
  }
  void visitAssign(AssignmentHandle assign) override {
    assign->lhs.accept(this);
    auto lhs = node;
    assign->rhs.accept(this);
    auto rhs = node;
    node = AssignmentNode::make("S" + IRNodeKeyGen::GetInstance()->yield(), lhs,
                                rhs);
  }
  void visitTensor(TensorHandle tensor) override { node = IRHandle(tensor); }
  void visitFor(ForHandle loop) override {
    loop->looping_var_.as<VarNode>()->min.accept(this);
    auto min = node;
    loop->looping_var_.as<VarNode>()->max.accept(this);
    auto max = node;
    loop->looping_var_.as<VarNode>()->increment.accept(this);
    auto increment = node;
    dict[loop->looping_var_.as<VarNode>()->id] = VarNode::make(
        "i" + IRNodeKeyGen::GetInstance()->yield(), min, max, increment);
    std::vector<IRHandle> body;
    for (int i = 0; i < loop->body.size(); i++) {
      loop->body[i].accept(this);
      body.push_back(node);
    }
    node = ForNode::make(dict[loop->looping_var_.as<VarNode>()->id]);
    node.as<ForNode>()->body = body;
  }
  void visitConst(ConstHandle con) override {
    throw std::runtime_error("Should Not visit this node");
  }
  void visitPrint(PrintHandle print) override {
    print->print.accept(this);
    node = PrintNode::make("S" + IRNodeKeyGen::GetInstance()->yield(), node);
  }
  void visitFunc(FuncHandle func) override {
    throw std::runtime_error("Should Not visit this node");
  }
};

IRHandle LoopSplit::create_remainder_loop(IRHandle loop,
                                          IRHandle remainder_loop_var) {
  // clone and replace statement/var/print ids
  LoopSplitHelper helper;
  helper.visit(loop);
  auto clonedLoop = helper.node;

  IRMutatorVisitor mutator(clonedLoop.as<ForNode>()->looping_var_,
                           remainder_loop_var, false);
  mutator.visit(clonedLoop);

  return clonedLoop;
}

}  // namespace polly