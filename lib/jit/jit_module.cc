#include "jit_module.h"

namespace polly {

void JitModule::visitInt(IntHandle int_expr) {
  v.int_value = int_expr->value;
  t = value_type::INT;
  return;
}

void JitModule::visitFloat(FloatHandle float_expr) {
  v.float_value = float_expr->value;
  t = value_type::FLOAT;
  return;
}

void JitModule::visitAdd(AddHandle add) {
  add->lhs.accept(this);
  value lhs = v;
  value_type lhs_type = t;
  add->rhs.accept(this);
  value rhs = v;
  value_type rhs_type = t;
  if (lhs_type != rhs_type) {
    throw std::runtime_error(
        "Jitter Interpret error: cannot add two expression of different  type");
  }
  switch (t) {
    case value_type::INT:
      v.int_value = lhs.int_value + rhs.int_value;
      break;
    case value_type::FLOAT:
      v.float_value = lhs.float_value + rhs.float_value;
      break;
    default:
      throw std::runtime_error("Unknown value type");
  }
}

void JitModule::visitSub(SubHandle sub) {
  sub->lhs.accept(this);
  value lhs = v;
  value_type lhs_type = t;
  sub->rhs.accept(this);
  value rhs = v;
  value_type rhs_type = t;
  if (lhs_type != rhs_type) {
    throw std::runtime_error(
        "Jitter Interpret error: cannot sub two expression of different  type");
  }
  switch (t) {
    case value_type::INT:
      v.int_value = lhs.int_value - rhs.int_value;
      break;
    case value_type::FLOAT:
      v.float_value = lhs.float_value - rhs.float_value;
      break;
    default:
      throw std::runtime_error("Unknown value type");
  }
}

void JitModule::visitMul(MulHandle mul) {
  mul->lhs.accept(this);
  value lhs = v;
  value_type lhs_type = t;
  mul->rhs.accept(this);
  value rhs = v;
  value_type rhs_type = t;
  if (lhs_type != rhs_type) {
    throw std::runtime_error(
        "Jitter Interpret error: cannot mul two expression of different  type");
  }
  switch (t) {
    case value_type::INT:
      v.int_value = lhs.int_value * rhs.int_value;
      break;
    case value_type::FLOAT:
      v.float_value = lhs.float_value * rhs.float_value;
      break;
    default:
      throw std::runtime_error("Unknown value type");
  }
}

void JitModule::visitDiv(DivHandle div) {
  div->lhs.accept(this);
  value lhs = v;
  value_type lhs_type = t;
  div->rhs.accept(this);
  value rhs = v;
  value_type rhs_type = t;
  if (lhs_type != rhs_type) {
    throw std::runtime_error(
        "Jitter Interpret error: cannot div two expression of different  type");
  }
  switch (t) {
    case value_type::INT:
      v.int_value = lhs.int_value / rhs.int_value;
      break;
    case value_type::FLOAT:
      v.float_value = lhs.float_value / rhs.float_value;
      break;
    default:
      throw std::runtime_error("Unknown value type");
  }
}

void JitModule::visitMod(ModHandle mod) {
  mod->lhs.accept(this);
  value lhs = v;
  value_type lhs_type = t;
  mod->rhs.accept(this);
  value rhs = v;
  value_type rhs_type = t;
  if (lhs_type != rhs_type) {
    throw std::runtime_error(
        "Jitter Interpret error: cannot mod two expression of different  type");
  }
  switch (t) {
    case value_type::INT:
      v.int_value = lhs.int_value % rhs.int_value;
      break;
    case value_type::FLOAT:
      throw std::runtime_error("Cannot use \% for two floating number");
      break;
    default:
      throw std::runtime_error("Unknown value type");
  }
}

void JitModule::visitVar(VarHandle var) {
  if (symbols_.find(var->id) == symbols_.end()) {
    // Initialize variable
    var->min.accept(this);
    assert(t == value_type::INT);
    symbols_[var->id] = v;
  }
  t = value_type::INT;
  v = symbols_[var->id];
}

void JitModule::visitAccess(AccessHandle access) {
  access->tensor.accept(this);
  int offset = 0;
  for (int i = 0; i < access->indices.size(); i++) {
    access->indices[i].accept(this);
    if (t != value_type::INT) {
      throw std::runtime_error("cannot access a tensor with float indices");
    }
    offset = offset * tensor_shapes_[access->tensor.as<TensorNode>()->id][i] +
             v.int_value;
  }
  t = value_type::FLOAT;
  v.float_value = *(tensor_ptr + offset);
}

void JitModule::visitAssign(AssignmentHandle assign) {
  assign->rhs.accept(this);
  float assigned_value = v.float_value;
  switch (assign->lhs.Type()) {
    case IRNodeType::ACCESS: {
      // Assert lhs must be a tensor access
      int offset = 0;

      auto access = assign->lhs.as<AccessNode>();

      for (int i = 0; i < access->indices.size(); i++) {
        access->indices[i].accept(this);
        if (t != value_type::INT) {
          throw std::runtime_error("cannot access a tensor with float indices");
        }
        offset =
            offset * tensor_shapes_[access->tensor.as<TensorNode>()->id][i] +
            v.int_value;
      }
      assign->lhs.as<AccessNode>()->tensor.accept(this);
      tensor_ptr[offset] = assigned_value;
      break;
    }
    case IRNodeType::VALUE: {
      symbols_[assign->lhs.as<ValNode>()->id].float_value = assigned_value;
      break;
    }
    default:
      break;
  }
}

void JitModule::visitTensor(TensorHandle tensor) {
  if (tensors_.find(tensor->id) == tensors_.end()) {
    tensor_shapes_[tensor->id] = {};
    size_t size = 1;
    for (int i = 0; i < tensor->shape.size(); i++) {
      tensor_shapes_[tensor->id].push_back(tensor->shape[i]);
      size *= tensor->shape[i];
    }
    tensors_[tensor->id] = new float[size];
    for (int i = 0; i < size; i++) tensors_[tensor->id][i] = .0;
  }
  tensor_ptr = tensors_[tensor->id];
}

void JitModule::visitVal(ValHandle val) {
  if (symbols_.find(val->id) == symbols_.end()) {
    // left the value undefined.
    symbols_[val->id].float_value = __FLT_MAX__;
  } else {
    t = value_type::FLOAT;
    v = symbols_[val->id];
  }
}

void JitModule::visitDecl(DeclHandle decl) { decl->decl.accept(this); }

void JitModule::visitFor(ForHandle loop) {
  loop->looping_var_.accept(this);
  VarHandle looping_var = loop->looping_var_.as<VarNode>();
  looping_var->max.accept(this);
  assert(t == value_type::INT);
  while (symbols_[looping_var->id].int_value < v.int_value) {
    for (int i = 0; i < loop->body.size(); i++) {
      loop->body[i].accept(this);
    }
    looping_var->increment.accept(this);
    assert(t == value_type::INT);
    symbols_[looping_var->id].int_value += v.int_value;
    looping_var->max.accept(this);
    assert(t == value_type::INT);
  }
  // when leave the for-loop scope, erase its var;
  symbols_.erase(looping_var->id);
}

void JitModule::visitConst(ConstHandle con) {
  throw std::runtime_error("Constant value is unknown");
}

void JitModule::visitPrint(PrintHandle print) {
  print->print.accept(this);
  switch (t) {
    case value_type::INT:
      std::cout << v.int_value << std::endl;
      break;
    case value_type::FLOAT:
      std::cout << v.float_value << std::endl;
      break;
    default:
      throw std::runtime_error("Cannot print an unknown type data");
  }
}

void JitModule::visitFunc(FuncHandle func) {
  for (int i = 0; i < func->body.size(); i++) {
    func->body[i].accept(this);
  }
}

void JitModule::visitMin(MinHandle min) {
  min->lhs.accept(this);
  value lhs = v;
  value_type lhs_type = t;
  min->rhs.accept(this);
  value rhs = v;
  value_type rhs_type = t;
  if (lhs_type != rhs_type) {
    throw std::runtime_error(
        "Jitter Interpret error: cannot min two expression of different type");
  }
  switch (t) {
    case value_type::INT:
      v.int_value = std::min(lhs.int_value, rhs.int_value);
      break;
    case value_type::FLOAT:
      v.float_value = std::min(lhs.float_value, rhs.float_value);
      break;
    default:
      throw std::runtime_error("Unknown value type");
  }
}

void JitModule::visitMax(MaxHandle max) {
  max->lhs.accept(this);
  value lhs = v;
  value_type lhs_type = t;
  max->rhs.accept(this);
  value rhs = v;
  value_type rhs_type = t;
  if (lhs_type != rhs_type) {
    throw std::runtime_error(
        "Jitter Interpret error: cannot max two expression of different type");
  }
  switch (t) {
    case value_type::INT:
      v.int_value = std::max(lhs.int_value, rhs.int_value);
      break;
    case value_type::FLOAT:
      v.float_value = std::max(lhs.float_value, rhs.float_value);
      break;
    default:
      throw std::runtime_error("Unknown value type");
  }
}

}  // namespace polly