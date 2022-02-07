#include "constant_folding.h"

namespace polly {

class ConstantFoldingEvaluator : public IRNotImplementedVisitor {
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

  ConstantFoldingEvaluator() : t(DEFAULT) {}
  value v;
  value_type t;

  IRHandle Evaluate(IRHandle expr) {
    t = value_type::DEFAULT;
    visit(expr);
    if (t == value_type::INT) {
      return IRHandle(IntNode::make(v.int_value));
    }
    /// add the case for float when FloatNode is added
    return NullIRHandle;
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

  void visitMin(MinHandle min) {
    t = value_type::DEFAULT;
    min->lhs.accept(this);
    auto lhs_t = t;
    auto lhs_v = v;
    if (t == value_type::DEFAULT) return;
    min->rhs.accept(this);
    if (t == value_type::DEFAULT) return;
    auto rhs_t = t;
    auto rhs_v = v;
    assert(lhs_t == rhs_t);
    switch (lhs_t) {
      case value_type::INT:
        v.int_value = std::min(lhs_v.int_value, rhs_v.int_value);
        break;
      case value_type::FLOAT:
        v.float_value = std::min(lhs_v.float_value, rhs_v.float_value);
        break;
      default:
        throw std::runtime_error("error!");
    }
  }

  void visitMax(MaxHandle max) {
    t = value_type::DEFAULT;
    max->lhs.accept(this);
    auto lhs_t = t;
    auto lhs_v = v;
    if (t == value_type::DEFAULT) return;
    max->rhs.accept(this);
    if (t == value_type::DEFAULT) return;
    auto rhs_t = t;
    auto rhs_v = v;
    assert(lhs_t == rhs_t);
    switch (lhs_t) {
      case value_type::INT:
        v.int_value = std::max(lhs_v.int_value, rhs_v.int_value);
        break;
      case value_type::FLOAT:
        v.float_value = std::max(lhs_v.float_value, rhs_v.float_value);
        break;
      default:
        throw std::runtime_error("error!");
    }
  }
};

IRHandle ConstantFoldingPass::simplifyMinMaxNode(IRHandle node) {
  bool already_folded = folded;
  if (!already_folded) folded = true;

  switch (node.Type()) {
    case IRNodeType::ADD: {
      if (node.as<AddNode>()->lhs.Type() == IRNodeType::MIN) {
        auto minNode = node.as<AddNode>()->lhs.as<MinNode>();
        return MinNode::make(
            AddNode::make(minNode->lhs, node.as<AddNode>()->rhs),
            AddNode::make(minNode->rhs, node.as<AddNode>()->rhs));
      }
      if (node.as<AddNode>()->lhs.Type() == IRNodeType::MAX) {
        auto maxNode = node.as<AddNode>()->lhs.as<MaxNode>();
        return MaxNode::make(
            AddNode::make(maxNode->lhs, node.as<AddNode>()->rhs),
            AddNode::make(maxNode->rhs, node.as<AddNode>()->rhs));
      }
      if (node.as<AddNode>()->rhs.Type() == IRNodeType::MIN) {
        auto minNode = node.as<AddNode>()->rhs.as<MinNode>();
        return MinNode::make(
            AddNode::make(minNode->lhs, node.as<AddNode>()->lhs),
            AddNode::make(minNode->rhs, node.as<AddNode>()->lhs));
      }
      if (node.as<AddNode>()->rhs.Type() == IRNodeType::MAX) {
        auto maxNode = node.as<AddNode>()->rhs.as<MaxNode>();
        return MaxNode::make(
            AddNode::make(maxNode->lhs, node.as<AddNode>()->lhs),
            AddNode::make(maxNode->rhs, node.as<AddNode>()->lhs));
      }
      break;
    }
    case IRNodeType::SUB: {
      if (node.as<SubNode>()->lhs.Type() == IRNodeType::MIN) {
        auto minNode = node.as<SubNode>()->lhs.as<MinNode>();
        return MinNode::make(
            SubNode::make(minNode->lhs, node.as<SubNode>()->rhs),
            SubNode::make(minNode->rhs, node.as<SubNode>()->rhs));
      }
      if (node.as<SubNode>()->lhs.Type() == IRNodeType::MAX) {
        auto maxNode = node.as<SubNode>()->lhs.as<MaxNode>();
        return MaxNode::make(
            SubNode::make(maxNode->lhs, node.as<SubNode>()->rhs),
            SubNode::make(maxNode->rhs, node.as<SubNode>()->rhs));
      }
      if (node.as<SubNode>()->rhs.Type() == IRNodeType::MIN) {
        auto minNode = node.as<SubNode>()->rhs.as<MinNode>();
        return MaxNode::make(
            SubNode::make(node.as<SubNode>()->lhs, minNode->lhs),
            SubNode::make(node.as<SubNode>()->lhs, minNode->rhs));
      }
      if (node.as<SubNode>()->rhs.Type() == IRNodeType::MAX) {
        auto maxNode = node.as<SubNode>()->rhs.as<MaxNode>();
        return MinNode::make(
            SubNode::make(node.as<SubNode>()->lhs, maxNode->lhs),
            SubNode::make(node.as<SubNode>()->lhs, maxNode->rhs));
      }
      break;
    }
    case IRNodeType::MUL: {
      if (node.as<MulNode>()->lhs.Type() == IRNodeType::MIN) {
        auto minNode = node.as<MulNode>()->lhs.as<MinNode>();
        return MinNode::make(
            MulNode::make(minNode->lhs, node.as<MulNode>()->rhs),
            MulNode::make(minNode->rhs, node.as<MulNode>()->rhs));
      }
      if (node.as<MulNode>()->lhs.Type() == IRNodeType::MAX) {
        auto maxNode = node.as<MulNode>()->lhs.as<MaxNode>();
        return MaxNode::make(
            MulNode::make(maxNode->lhs, node.as<MulNode>()->rhs),
            MulNode::make(maxNode->rhs, node.as<MulNode>()->rhs));
      }
      if (node.as<MulNode>()->rhs.Type() == IRNodeType::MIN) {
        auto minNode = node.as<MulNode>()->rhs.as<MinNode>();
        return MinNode::make(
            MulNode::make(minNode->lhs, node.as<MulNode>()->lhs),
            MulNode::make(minNode->rhs, node.as<MulNode>()->lhs));
      }
      if (node.as<MulNode>()->rhs.Type() == IRNodeType::MAX) {
        auto maxNode = node.as<MulNode>()->rhs.as<MaxNode>();
        return MaxNode::make(
            MulNode::make(maxNode->lhs, node.as<MulNode>()->lhs),
            MulNode::make(maxNode->rhs, node.as<MulNode>()->lhs));
      }
      break;
    }
    case IRNodeType::DIV: {
      if (node.as<DivNode>()->lhs.Type() == IRNodeType::MIN) {
        auto minNode = node.as<DivNode>()->lhs.as<MinNode>();
        return MinNode::make(
            DivNode::make(minNode->lhs, node.as<DivNode>()->rhs),
            DivNode::make(minNode->rhs, node.as<DivNode>()->rhs));
      }
      if (node.as<DivNode>()->lhs.Type() == IRNodeType::MAX) {
        auto maxNode = node.as<DivNode>()->lhs.as<MaxNode>();
        return MaxNode::make(
            DivNode::make(maxNode->lhs, node.as<DivNode>()->rhs),
            DivNode::make(maxNode->rhs, node.as<DivNode>()->rhs));
      }
      break;
    }
    case IRNodeType::MOD: {
      if (node.as<ModNode>()->lhs.Type() == IRNodeType::MIN) {
        auto minNode = node.as<ModNode>()->lhs.as<MinNode>();
        return MinNode::make(
            ModNode::make(minNode->lhs, node.as<ModNode>()->rhs),
            ModNode::make(minNode->rhs, node.as<ModNode>()->rhs));
      }
      if (node.as<ModNode>()->lhs.Type() == IRNodeType::MAX) {
        auto maxNode = node.as<ModNode>()->lhs.as<MaxNode>();
        return MaxNode::make(
            ModNode::make(maxNode->lhs, node.as<ModNode>()->rhs),
            ModNode::make(maxNode->rhs, node.as<ModNode>()->rhs));
      }
      break;
    }
    default:
      break;
  }

  if (!already_folded) folded = false;
  return node;
}

IRHandle ConstantFoldingPass::simplify(IRHandle node) {
  node = simplifyMinMaxNode(node);

  bool already_folded = folded;
  if (!already_folded) folded = true;
  auto zero = IntNode::make(0);
  auto one = IntNode::make(1);
  switch (node.Type()) {
    case IRNodeType::ADD: {
      if (node.as<AddNode>()->lhs.equals(zero)) return node.as<AddNode>()->rhs;
      if (node.as<AddNode>()->rhs.equals(zero)) return node.as<AddNode>()->lhs;
      break;
    }
    case IRNodeType::SUB: {
      if (node.as<SubNode>()->rhs.equals(zero)) return node.as<SubNode>()->lhs;
      break;
    }
    case IRNodeType::MUL: {
      if (node.as<MulNode>()->lhs.equals(zero)) return zero;
      if (node.as<MulNode>()->rhs.equals(zero)) return zero;
      if (node.as<MulNode>()->lhs.equals(one)) return node.as<MulNode>()->rhs;
      if (node.as<MulNode>()->rhs.equals(one)) return node.as<MulNode>()->lhs;
      break;
    }
    case IRNodeType::DIV: {
      if (node.as<DivNode>()->rhs.equals(one)) return node.as<DivNode>()->lhs;
      break;
    }
    case IRNodeType::MOD: {
      if (node.as<ModNode>()->rhs.equals(one)) zero;
      break;
    }
    default:
      break;
  }
  if (!already_folded) folded = false;
  return node;
}

void ConstantFoldingPass::visitInt(IntHandle int_expr) {
  /// Pass
}

void ConstantFoldingPass::visitAdd(AddHandle add) {
  ConstantFoldingEvaluator evaluator;
  add->rhs.accept(this);
  IRHandle lhs = evaluator.Evaluate(add->lhs);
  if (lhs != NullIRHandle) {
    add->lhs = lhs;
  }
  add->lhs.accept(this);
  IRHandle rhs = evaluator.Evaluate(add->rhs);
  if (rhs != NullIRHandle) {
    add->rhs = rhs;
  }
  add->lhs = simplify(add->lhs);
  add->rhs = simplify(add->rhs);
}

void ConstantFoldingPass::visitSub(SubHandle sub) {
  ConstantFoldingEvaluator evaluator;
  sub->rhs.accept(this);
  IRHandle lhs = evaluator.Evaluate(sub->lhs);
  if (lhs != NullIRHandle) {
    sub->lhs = lhs;
  }
  sub->lhs.accept(this);
  IRHandle rhs = evaluator.Evaluate(sub->rhs);
  if (rhs != NullIRHandle) {
    sub->rhs = rhs;
  }
  sub->lhs = simplify(sub->lhs);
  sub->rhs = simplify(sub->rhs);
}

void ConstantFoldingPass::visitMul(MulHandle mul) {
  ConstantFoldingEvaluator evaluator;
  mul->rhs.accept(this);
  IRHandle lhs = evaluator.Evaluate(mul->lhs);
  if (lhs != NullIRHandle) {
    mul->lhs = lhs;
  }
  mul->lhs.accept(this);
  IRHandle rhs = evaluator.Evaluate(mul->rhs);
  if (rhs != NullIRHandle) {
    mul->rhs = rhs;
  }
  mul->lhs = simplify(mul->lhs);
  mul->rhs = simplify(mul->rhs);
}

void ConstantFoldingPass::visitDiv(DivHandle div) {
  ConstantFoldingEvaluator evaluator;
  div->rhs.accept(this);
  IRHandle lhs = evaluator.Evaluate(div->lhs);
  if (lhs != NullIRHandle) {
    div->lhs = lhs;
  }
  div->lhs.accept(this);
  IRHandle rhs = evaluator.Evaluate(div->rhs);
  if (rhs != NullIRHandle) {
    div->rhs = rhs;
  }
  div->lhs = simplify(div->lhs);
  div->rhs = simplify(div->rhs);
}

void ConstantFoldingPass::visitMod(ModHandle mod) {
  ConstantFoldingEvaluator evaluator;
  mod->rhs.accept(this);
  IRHandle lhs = evaluator.Evaluate(mod->lhs);
  if (lhs != NullIRHandle) {
    mod->lhs = lhs;
  }
  mod->lhs.accept(this);
  IRHandle rhs = evaluator.Evaluate(mod->rhs);
  if (rhs != NullIRHandle) {
    mod->rhs = rhs;
  }
  mod->lhs = simplify(mod->lhs);
  mod->rhs = simplify(mod->rhs);
}

void ConstantFoldingPass::visitVar(VarHandle var) {
  ConstantFoldingEvaluator evaluator;
  var->min.accept(this);
  IRHandle min = evaluator.Evaluate(var->min);
  if (min != NullIRHandle) {
    var->min = min;
  }
  var->max.accept(this);
  IRHandle max = evaluator.Evaluate(var->max);
  if (max != NullIRHandle) {
    var->max = max;
  }
  var->increment.accept(this);
  IRHandle increment = evaluator.Evaluate(var->increment);
  if (increment != NullIRHandle) {
    var->increment = increment;
  }
  var->min = simplify(var->min);
  var->max = simplify(var->max);
  var->increment = simplify(var->increment);
}

void ConstantFoldingPass::visitAccess(AccessHandle access) {
  ConstantFoldingEvaluator evaluator;
  for (int i = 0; i < access->indices.size(); i++) {
    access->indices[i].accept(this);
    auto ind = evaluator.Evaluate(access->indices[i]);
    if (ind != NullIRHandle) {
      access->indices[i] = ind;
    }
    access->indices[i] = simplify(access->indices[i]);
  }
}

void ConstantFoldingPass::visitAssign(AssignmentHandle assign) {
  ConstantFoldingEvaluator evaluator;
  assign->lhs.accept(this);
  assign->rhs.accept(this);
  IRHandle rhs = evaluator.Evaluate(assign->rhs);
  if (rhs != NullIRHandle) {
    assign->rhs = rhs;
  }
  assign->lhs = simplify(assign->lhs);
  assign->rhs = simplify(assign->rhs);
}

void ConstantFoldingPass::visitTensor(TensorHandle tensor) {
  /// Pass
}

void ConstantFoldingPass::visitFor(ForHandle loop) {
  loop->looping_var_.accept(this);
  for (int i = 0; i < loop->body.size(); i++) {
    loop->body[i].accept(this);
  }
}

void ConstantFoldingPass::visitConst(ConstHandle con) {
  /// Pass
}

void ConstantFoldingPass::visitPrint(PrintHandle print) {
  ConstantFoldingEvaluator evaluator;
  print->print.accept(this);
  auto pr = evaluator.Evaluate(print->print);
  if (pr != NullIRHandle) {
    print->print = pr;
  }
  print->print = simplify(print->print);
}

void ConstantFoldingPass::visitFunc(FuncHandle func) {
  for (int i = 0; i < func->body.size(); i++) {
    func->body[i].accept(this);
  }
}

void ConstantFoldingPass::visitMin(MinHandle min) {
  ConstantFoldingEvaluator evaluator;
  min->rhs.accept(this);
  IRHandle lhs = evaluator.Evaluate(min->lhs);
  if (lhs != NullIRHandle) {
    min->lhs = lhs;
  }
  min->lhs.accept(this);
  IRHandle rhs = evaluator.Evaluate(min->rhs);
  if (rhs != NullIRHandle) {
    min->rhs = rhs;
  }
  min->lhs = simplify(min->lhs);
  min->rhs = simplify(min->rhs);
}

void ConstantFoldingPass::visitMax(MaxHandle max) {
  ConstantFoldingEvaluator evaluator;
  max->rhs.accept(this);
  IRHandle lhs = evaluator.Evaluate(max->lhs);
  if (lhs != NullIRHandle) {
    max->lhs = lhs;
  }
  max->lhs.accept(this);
  IRHandle rhs = evaluator.Evaluate(max->rhs);
  if (rhs != NullIRHandle) {
    max->rhs = rhs;
  }
  max->lhs = simplify(max->lhs);
  max->rhs = simplify(max->rhs);
}

}  // namespace polly