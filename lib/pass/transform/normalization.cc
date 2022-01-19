#include "normalization.h"

namespace polly {

PassRetHandle NormalizationPass::runPass(PassArgHandle arg) {
  NormalizationPass(PassArg::as<Arg>(arg)->program);
  return Ret::create();
}

class NormalizationHelper : public IRVisitor {
 public:
  NormalizationHelper(IRHandle program, IRHandle loop_var, IRHandle replace_var)
      : program_(program), loop_var_(loop_var), replace_var_(replace_var) {}

  static void Normalize(IRHandle program, IRHandle loop_var,
                        IRHandle replace_var) {
    NormalizationHelper norm(program, loop_var, replace_var);
    norm.visit(program);
  }

  void visitInt(IntHandle int_expr) override {
    /// Pass
  }
  void visitAdd(AddHandle add) override {
    add->lhs = replace_with(add->lhs);
    add->rhs = replace_with(add->rhs);
  }
  void visitSub(SubHandle sub) override {
    sub->lhs = replace_with(sub->lhs);
    sub->rhs = replace_with(sub->rhs);
  }
  void visitMul(MulHandle mul) override {
    mul->lhs = replace_with(mul->lhs);
    mul->rhs = replace_with(mul->rhs);
  }
  void visitDiv(DivHandle div) override {
    div->lhs = replace_with(div->lhs);
    div->rhs = replace_with(div->rhs);
  }
  void visitMod(ModHandle mod) override {
    mod->lhs = replace_with(mod->lhs);
    mod->rhs = replace_with(mod->rhs);
  }
  void visitVar(VarHandle var) override {
    var->min = replace_with(var->min);
    var->max = replace_with(var->max);
    var->increment = replace_with(var->increment);
  }
  void visitAccess(AccessHandle access) override {
    for (int i = 0; i < access->indices.size(); i++) {
      access->indices[i] = replace_with(access->indices[i]);
    }
  }
  void visitAssign(AssignmentHandle assign) override {
    assign->lhs = replace_with(assign->lhs);
    assign->rhs = replace_with(assign->rhs);
  }
  void visitTensor(TensorHandle tensor) override {
    /// Pass
  }
  void visitFor(ForHandle loop) override {
    loop->looping_var_ = replace_with(loop->looping_var_);
    for (int i = 0; i < loop->body.size(); i++) {
      loop->body[i] = replace_with(loop->body[i]);
    }
  }
  void visitConst(ConstHandle con) override {
    /// Pass
  }
  void visitPrint(PrintHandle print) override {
    print->print = replace_with(print->print);
  }
  void visitFunc(FuncHandle func) override {
    for (int i = 0; i < func->body.size(); i++) {
      func->body[i] = replace_with(func->body[i]);
    }
  }

  IRHandle replace_with(IRHandle handle) {
    if (handle.equals(loop_var_)) return replace_var_;
    handle.accept(this);
    return handle;
  }

  IRHandle program_;
  IRHandle loop_var_;
  IRHandle replace_var_;
};

void NormalizationPass::visitInt(IntHandle int_expr) {
  /// Pass
}

void NormalizationPass::visitAdd(AddHandle add) {
  /// Pass
}

void NormalizationPass::visitSub(SubHandle sub) {
  /// Pass
}

void NormalizationPass::visitMul(MulHandle mul) {
  /// Pass
}

void NormalizationPass::visitDiv(DivHandle div) {
  /// Pass
}

void NormalizationPass::visitMod(ModHandle mod) {
  /// Pass
}

void NormalizationPass::visitVar(VarHandle var) {
  /// Pass
}

void NormalizationPass::visitAccess(AccessHandle access) {
  /// Pass
}

void NormalizationPass::visitAssign(AssignmentHandle assign) {
  /// Pass
}

void NormalizationPass::visitTensor(TensorHandle tensor) {
  /// Pass
}

void NormalizationPass::visitFor(ForHandle loop) {
  for (int i = 0; i < loop->body.size(); i++) {
    if (loop->body[i].Type() == IRNodeType::FOR) {
      auto var = loop->body[i].as<ForNode>()->looping_var_;
      if (!isNormalized(var)) {
        IRHandle newVar = VarNode::make(
            "i" + IRNodeKeyGen::GetInstance()->yield(), IntNode::make(0),
            DivNode::make(
                SubNode::make(
                    AddNode::make(var.as<VarNode>()->max,
                                  SubNode::make(var.as<VarNode>()->increment,
                                                IntNode::make(1))),
                    var.as<VarNode>()->min),
                var.as<VarNode>()->increment),
            IntNode::make(1));
        loop->body[i].as<ForNode>()->looping_var_ = newVar;
        NormalizationHelper::Normalize(
            loop->body[i], var,
            AddNode::make(MulNode::make(var.as<VarNode>()->increment, newVar),
                          var.as<VarNode>()->min));
      }
    }
  }
}

void NormalizationPass::visitConst(ConstHandle con) {
  /// Pass
}

void NormalizationPass::visitPrint(PrintHandle print) {
  /// Pass
}

void NormalizationPass::visitFunc(FuncHandle func) {
  for (int i = 0; i < func->body.size(); i++) {
    if (func->body[i].Type() == IRNodeType::FOR) {
      auto var = func->body[i].as<ForNode>()->looping_var_;
      if (!isNormalized(var)) {
        IRHandle newVar = VarNode::make(
            "i" + IRNodeKeyGen::GetInstance()->yield(), IntNode::make(0),
            DivNode::make(
                SubNode::make(
                    AddNode::make(var.as<VarNode>()->max,
                                  SubNode::make(var.as<VarNode>()->increment,
                                                IntNode::make(1))),
                    var.as<VarNode>()->min),
                var.as<VarNode>()->increment),
            IntNode::make(1));
        func->body[i].as<ForNode>()->looping_var_ = newVar;
        NormalizationHelper::Normalize(
            func->body[i], var,
            AddNode::make(MulNode::make(var.as<VarNode>()->increment, newVar),
                          var.as<VarNode>()->min));
      }
    }
  }
}

}  // namespace polly