#include "constant_folding.h"

namespace polly {

class ConstantFoldingEvaluator : public IRVisitor {
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
};

IRHandle ConstantFoldingPass::simplify(IRHandle node) {
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

}  // namespace polly