#include "unroll.h"

namespace polly {

class UnrollEvaluator : public IRVisitor {
 public:
  union value {
    int int_value;
    float float_value;
  };

  enum value_type {
    INT,
    FLOAT,
    DEFAULT,
  };

  UnrollEvaluator() : t(DEFAULT) {}
  value v;
  value_type t;

  value Evaluate(IRHandle expr) {
    t = value_type::DEFAULT;
    visit(expr);
    if (t == value_type::INT) {
      return v;
    }
    throw std::runtime_error("Unroll Evaluator evluates a non-int value");
  }

  void visitInt(IntHandle int_expr) {
    t = value_type::INT;
    v.int_value = int_expr->value;
  }

  void visitAdd(AddHandle add) {
    t = value_type::DEFAULT;
    add->lhs.accept(this);
    auto lhs_t = t;
    auto lhs_v = v;
    if (t == value_type::DEFAULT) return;
    add->rhs.accept(this);
    if (t == value_type::DEFAULT) return;
    auto rhs_t = t;
    auto rhs_v = v;
    assert(lhs_t == rhs_t);
    switch (lhs_t) {
      case value_type::INT:
        v.int_value = lhs_v.int_value + rhs_v.int_value;
        break;
      case value_type::FLOAT:
        v.float_value = lhs_v.float_value + rhs_v.float_value;
        break;
      default:
        throw std::runtime_error("error!");
    }
  }

  void visitSub(SubHandle sub) {
    t = value_type::DEFAULT;
    sub->lhs.accept(this);
    auto lhs_t = t;
    auto lhs_v = v;
    if (t == value_type::DEFAULT) return;
    sub->rhs.accept(this);
    if (t == value_type::DEFAULT) return;
    auto rhs_t = t;
    auto rhs_v = v;
    assert(lhs_t == rhs_t);
    switch (lhs_t) {
      case value_type::INT:
        v.int_value = lhs_v.int_value - rhs_v.int_value;
        break;
      case value_type::FLOAT:
        v.float_value = lhs_v.float_value - rhs_v.float_value;
        break;
      default:
        throw std::runtime_error("error!");
    }
  }

  void visitMul(MulHandle mul) {
    t = value_type::DEFAULT;
    mul->lhs.accept(this);
    auto lhs_t = t;
    auto lhs_v = v;
    if (t == value_type::DEFAULT) return;
    mul->rhs.accept(this);
    if (t == value_type::DEFAULT) return;
    auto rhs_t = t;
    auto rhs_v = v;
    assert(lhs_t == rhs_t);
    switch (lhs_t) {
      case value_type::INT:
        v.int_value = lhs_v.int_value * rhs_v.int_value;
        break;
      case value_type::FLOAT:
        v.float_value = lhs_v.float_value * rhs_v.float_value;
        break;
      default:
        throw std::runtime_error("error!");
    }
  }

  void visitDiv(DivHandle div) {
    t = value_type::DEFAULT;
    div->lhs.accept(this);
    auto lhs_t = t;
    auto lhs_v = v;
    if (t == value_type::DEFAULT) return;
    div->rhs.accept(this);
    if (t == value_type::DEFAULT) return;
    auto rhs_t = t;
    auto rhs_v = v;
    assert(lhs_t == rhs_t);
    switch (lhs_t) {
      case value_type::INT:
        v.int_value = lhs_v.int_value / rhs_v.int_value;
        break;
      case value_type::FLOAT:
        v.float_value = lhs_v.float_value / rhs_v.float_value;
        break;
      default:
        throw std::runtime_error("error!");
    }
  }

  void visitMod(ModHandle mod) {
    t = value_type::DEFAULT;
    mod->lhs.accept(this);
    auto lhs_t = t;
    auto lhs_v = v;
    if (t == value_type::DEFAULT) return;
    mod->rhs.accept(this);
    if (t == value_type::DEFAULT) return;
    auto rhs_t = t;
    auto rhs_v = v;
    assert(lhs_t == rhs_t);
    switch (lhs_t) {
      case value_type::INT:
        v.int_value = lhs_v.int_value % rhs_v.int_value;
        break;
      case value_type::FLOAT:
        throw std::runtime_error("error!");
        break;
      default:
        throw std::runtime_error("error!");
    }
  }

  void visitVar(VarHandle var) { t = value_type::DEFAULT; }
  void visitAccess(AccessHandle access) { t = value_type::DEFAULT; }
  void visitAssign(AssignmentHandle assign) { t = value_type::DEFAULT; }
  void visitTensor(TensorHandle tensor) { t = value_type::DEFAULT; }
  void visitFor(ForHandle loop) { t = value_type::DEFAULT; }
  void visitConst(ConstHandle con) { t = value_type::DEFAULT; }
  void visitPrint(PrintHandle print) { t = value_type::DEFAULT; }
  void visitFunc(FuncHandle func) { t = value_type::DEFAULT; }
};

void LoopUnroll::visitInt(IntHandle int_expr) {
  tape_.push(IntNode::make(int_expr->value));
}

void LoopUnroll::visitAdd(AddHandle add) {
  add->lhs.accept(this);
  add->rhs.accept(this);
  auto rhs = tape_.top();
  tape_.pop();
  auto lhs = tape_.top();
  tape_.pop();
  tape_.push(AddNode::make(lhs, rhs));
}

void LoopUnroll::visitSub(SubHandle sub) {
  sub->lhs.accept(this);
  sub->rhs.accept(this);
  auto rhs = tape_.top();
  tape_.pop();
  auto lhs = tape_.top();
  tape_.pop();
  tape_.push(SubNode::make(lhs, rhs));
}

void LoopUnroll::visitMul(MulHandle mul) {
  mul->lhs.accept(this);
  mul->rhs.accept(this);
  auto rhs = tape_.top();
  tape_.pop();
  auto lhs = tape_.top();
  tape_.pop();
  tape_.push(MulNode::make(lhs, rhs));
}

void LoopUnroll::visitDiv(DivHandle div) {
  div->lhs.accept(this);
  div->rhs.accept(this);
  auto rhs = tape_.top();
  tape_.pop();
  auto lhs = tape_.top();
  tape_.pop();
  tape_.push(DivNode::make(lhs, rhs));
}

void LoopUnroll::visitMod(ModHandle mod) {
  mod->lhs.accept(this);
  mod->rhs.accept(this);
  auto rhs = tape_.top();
  tape_.pop();
  auto lhs = tape_.top();
  tape_.pop();
  tape_.push(ModNode::make(lhs, rhs));
}

void LoopUnroll::visitVar(VarHandle var) {
  if (IRHandle(var).equals(this->var)) {
    tape_.push(int_expr);
  } else {
    tape_.push(VarNode::make(var->name, var->min, var->max, var->increment));
  }
}

void LoopUnroll::visitAccess(AccessHandle access) {
  access->tensor.accept(this);
  auto tensor = tape_.top();
  tape_.pop();
  std::vector<IRHandle> indices;
  for (int i = 0; i < access->indices.size(); i++) {
    access->indices[i].accept(this);
    auto ind = tape_.top();
    indices.push_back(ind);
    tape_.pop();
  }
  tape_.push(AccessNode::make(tensor, indices));
}

void LoopUnroll::visitAssign(AssignmentHandle assign) {
  assign->lhs.accept(this);
  assign->rhs.accept(this);
  auto rhs = tape_.top();
  tape_.pop();
  auto lhs = tape_.top();
  tape_.pop();
  tape_.push(AssignmentNode::make(lhs, rhs));
}

void LoopUnroll::visitTensor(TensorHandle tensor) {
  tape_.push(TensorNode::make(tensor->name, tensor->shape));
}

void LoopUnroll::visitFor(ForHandle loop) {
  bool isInnerMostLoop = true;
  for (int i = 0; i < loop->body.size(); i++) {
    if (loop->body[i].Type() == IRNodeType::FOR) {
      isInnerMostLoop = false;
      loop->body[i].accept(this);
      if (reachInnerMostLoop) {
        reachInnerMostLoop = false;
        auto innerLoop = loop->body[i].as<ForNode>();
        for (int j = 0; j < innerLoop->body.size(); j++) {
          loop->body.insert(loop->body.begin() + i + j, innerLoop->body[j]);
        }
        loop->body.erase(loop->body.begin() + i + innerLoop->body.size());
      }
    }
  }

  if (isInnerMostLoop) {
    std::vector<IRHandle> unrolledBody;
    UnrollEvaluator evaluator;
    auto min =
        evaluator.Evaluate(loop->looping_var_.as<VarNode>()->min).int_value;
    auto max =
        evaluator.Evaluate(loop->looping_var_.as<VarNode>()->max).int_value;
    auto increment =
        evaluator.Evaluate(loop->looping_var_.as<VarNode>()->increment)
            .int_value;
    for (int i = min; i < max; i += increment) {
      for (int ind = 0; ind < loop->body.size(); ind++) {
        unrolledBody.push_back(replaceVarWithInt(
            loop->body[ind], loop->looping_var_, IntNode::make(i)));
      }
    }
    loop->body = unrolledBody;
    reachInnerMostLoop = true;
  }
}

void LoopUnroll::visitConst(ConstHandle con) {
  tape_.push(ConstNode::make(con->name));
}

void LoopUnroll::visitPrint(PrintHandle print) {
  print->print.accept(this);
  auto pr = tape_.top();
  tape_.pop();
  tape_.push(PrintNode::make(pr));
}

void LoopUnroll::visitFunc(FuncHandle func) {
  for (int i = 0; i < func->body.size(); i++) {
    func->body[i].accept(this);
  }
}

IRHandle LoopUnroll::replaceVarWithInt(IRHandle node, IRHandle var,
                                       IRHandle int_expr) {
  assert(tape_.size() == 0);
  this->var = var;
  this->int_expr = int_expr;
  this->visit(node);
  IRHandle ret = tape_.top();
  assert(tape_.size() == 1);
  tape_.pop();
  return ret;
}

}  // namespace polly