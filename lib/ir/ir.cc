#include "ir.h"
#include "ir_visitor.h"

namespace polly {

IRNodeKeyGen *IRNodeKeyGen::GetInstance() {
  if (generator == nullptr) generator = new IRNodeKeyGen;
  return generator;
}

IRNodeKeyGen *IRNodeKeyGen::generator = nullptr;

IRHandle IRHandle::clone(std::map<IRNodeKey, IRHandle> &irHandleDict) {
  if (isNull()) return IRHandle();
  IRHandle ret;
  switch (Type()) {
    case IRNodeType::ADD: {
      ret = AddNode::make(as<AddNode>()->lhs.clone(irHandleDict),
                          as<AddNode>()->rhs.clone(irHandleDict));
      break;
    }
    case IRNodeType::SUB: {
      ret = SubNode::make(as<SubNode>()->lhs.clone(irHandleDict),
                          as<SubNode>()->rhs.clone(irHandleDict));
      break;
    }
    case IRNodeType::MUL: {
      ret = MulNode::make(as<MulNode>()->lhs.clone(irHandleDict),
                          as<MulNode>()->rhs.clone(irHandleDict));
      break;
    }
    case IRNodeType::DIV: {
      ret = DivNode::make(as<DivNode>()->lhs.clone(irHandleDict),
                          as<DivNode>()->rhs.clone(irHandleDict));
      break;
    }
    case IRNodeType::MOD: {
      ret = ModNode::make(as<ModNode>()->lhs.clone(irHandleDict),
                          as<ModNode>()->rhs.clone(irHandleDict));
      break;
    }
    case IRNodeType::TENSOR: {
      if (irHandleDict.find(as<TensorNode>()->id) != irHandleDict.end()) {
        ret = irHandleDict[as<TensorNode>()->id];
      } else {
        ret = TensorNode::make(as<TensorNode>()->id, as<TensorNode>()->shape);
        irHandleDict.insert(std::make_pair(as<TensorNode>()->id, ret));
      }
      break;
    }
    case IRNodeType::VAR: {
      if (irHandleDict.find(as<VarNode>()->id) != irHandleDict.end()) {
        ret = irHandleDict[as<VarNode>()->id];
      } else {
        ret = VarNode::make(as<VarNode>()->id,
                            as<VarNode>()->min.clone(irHandleDict),
                            as<VarNode>()->max.clone(irHandleDict),
                            as<VarNode>()->increment.clone(irHandleDict));
        irHandleDict.insert(std::make_pair(as<VarNode>()->id, ret));
      }
      break;
    }
    case IRNodeType::ACCESS: {
      std::vector<IRHandle> indices;
      auto ind = as<AccessNode>()->indices;
      std::transform(ind.begin(), ind.end(), std::back_inserter(indices),
                     [&](IRHandle handle) -> IRHandle {
                       return handle.clone(irHandleDict);
                     });
      ret = AccessNode::make(as<AccessNode>()->tensor.clone(irHandleDict),
                             indices);
      break;
    }
    case IRNodeType::ASSIGN: {
      ret = AssignmentNode::make(as<AssignmentNode>()->id,
                                 as<AssignmentNode>()->lhs.clone(irHandleDict),
                                 as<AssignmentNode>()->rhs.clone(irHandleDict));
      break;
    }
    case IRNodeType::CONST: {
      if (irHandleDict.find(as<ConstNode>()->name) != irHandleDict.end()) {
        ret = irHandleDict[as<ConstNode>()->name];
      } else {
        ret = ConstNode::make(as<ConstNode>()->name);
        irHandleDict.insert(std::make_pair(as<ConstNode>()->name, ret));
      }
      break;
    }
    case IRNodeType::FOR: {
      // For node name: "for"-`var-name`
      if (irHandleDict.find("For-" +
                            as<ForNode>()->looping_var_.as<VarNode>()->id) !=
          irHandleDict.end()) {
        ret = irHandleDict["For-" +
                           as<ForNode>()->looping_var_.as<VarNode>()->id];
      } else {
        ret = ForNode::make(as<ForNode>()->looping_var_.clone(irHandleDict));
        irHandleDict.insert(std::make_pair(
            "For-" + as<ForNode>()->looping_var_.as<VarNode>()->id, ret));
        auto forNode = as<ForNode>();
        for (int i = 0; i < forNode->body.size(); i++) {
          ret.as<ForNode>()->Insert(forNode->body[i].clone(irHandleDict));
        }
      }
      break;
    }
    case IRNodeType::INT: {
      ret = IntNode::make(as<IntNode>()->value);
      break;
    }
    case IRNodeType::PRINT: {
      ret = PrintNode::make(as<PrintNode>()->id,
                            as<PrintNode>()->print.clone(irHandleDict));
      break;
    }
    case IRNodeType::FUNC: {
      std::vector<IRHandle> body;
      for (int i = 0; i < as<FuncNode>()->body.size(); i++) {
        body.push_back(as<FuncNode>()->body[i].clone(irHandleDict));
      }
      ret = FuncNode::make(body);
      break;
    }

    default:
      throw std::runtime_error("Unknown IRHandle Type, cannot clone");
  }
  return ret;
}

IRHandle AddNode::make(IRHandle lhs, IRHandle rhs) {
  AddNode *add = new AddNode();
  add->lhs = lhs;
  add->rhs = rhs;
  return IRHandle(add);
}

IRHandle SubNode::make(IRHandle lhs, IRHandle rhs) {
  SubNode *sub = new SubNode();
  sub->lhs = lhs;
  sub->rhs = rhs;
  return IRHandle(sub);
}

IRHandle MulNode::make(IRHandle lhs, IRHandle rhs) {
  MulNode *mul = new MulNode();
  mul->lhs = lhs;
  mul->rhs = rhs;
  return IRHandle(mul);
}

IRHandle DivNode::make(IRHandle lhs, IRHandle rhs) {
  DivNode *div = new DivNode();
  div->lhs = lhs;
  div->rhs = rhs;
  return IRHandle(div);
}

IRHandle ModNode::make(IRHandle lhs, IRHandle rhs) {
  ModNode *div = new ModNode();
  div->lhs = lhs;
  div->rhs = rhs;
  return IRHandle(div);
}

IRHandle VarNode::make(const IRNodeKey id, IRHandle min, IRHandle max,
                       IRHandle increment) {
  VarNode *var = new VarNode();
  var->min = min;
  var->max = max;
  var->increment = increment;
  var->id = id;
  return IRHandle(var);
}

IRHandle IntNode::make(int x) {
  IntNode *node = new IntNode(x);
  return IRHandle(node);
}

IRHandle TensorNode::make(const IRNodeKey id, std::vector<int64_t> &shape) {
  TensorNode *tensor = new TensorNode();
  tensor->id = id;
  tensor->shape = shape;
  return IRHandle(tensor);
}

IRHandle AccessNode::make(IRHandle tensor, std::vector<IRHandle> indices) {
  AccessNode *node = new AccessNode;
  node->tensor = tensor;
  node->indices = indices;
  return IRHandle(node);
}

IRHandle AssignmentNode::make(IRNodeKey id, IRHandle lhs, IRHandle rhs) {
  AssignmentNode *node = new AssignmentNode();
  node->lhs = lhs;
  node->rhs = rhs;
  node->id = id;
  return IRHandle(node);
}

IRHandle ForNode::make(IRHandle looping_var) {
  ForNode *node = new ForNode();
  node->looping_var_ = looping_var;
  return IRHandle(node);
}

IRHandle ConstNode::make(std::string name) {
  // ConstNode *node = new ConstNode();
  // node->name = name;
  // return IRHandle(node);
  throw std::runtime_error("Not Implemented Error: ConstNode");
}

IRHandle PrintNode::make(IRNodeKey id, IRHandle print) {
  PrintNode *node = new PrintNode();
  node->print = print;
  node->id = id;
  return IRHandle(node);
}

IRHandle FuncNode::make(std::vector<IRHandle> body) {
  FuncNode *node = new FuncNode();
  node->body = body;
  return IRHandle(node);
}

void IRHandle::accept(IRVisitor *visitor) { visitor->visit(*this); }

IRHandle VecNode::make(IRNodeKey id, int length) {
  VecNode *node = new VecNode();
  node->id = id;
  node->length = length;
  return IRHandle(node);
}

IRHandle VecScalarNode::make(IRHandle vec, IRHandle scalar, int length) {
  VecScalarNode *node = new VecScalarNode();
  node->length = length;
  node->vec = vec;
  node->scalar = scalar;
  return IRHandle(node);
}

IRHandle VecLoadNode::make(IRHandle vec, IRHandle data, int length) {
  VecLoadNode *node = new VecLoadNode();
  node->vec = vec;
  node->data = data;
  node->length = length;
  return IRHandle(node);
}

IRHandle VecBroadCastLoadNode::make(IRHandle vec, IRHandle data, int length) {
  VecBroadCastLoadNode *node = new VecBroadCastLoadNode();
  node->vec = vec;
  node->data = data;
  node->length = length;
  return IRHandle(node);
}

IRHandle VecStoreNode::make(IRHandle vec, IRHandle data, int length) {
  VecStoreNode *node = new VecStoreNode();
  node->vec = vec;
  node->data = data;
  node->length = length;
  return IRHandle(node);
}

IRHandle VecAddNode::make(IRHandle vec, IRHandle lhs, IRHandle rhs,
                          int length) {
  VecAddNode *node = new VecAddNode();
  node->vec = vec;
  node->lhs = lhs;
  node->rhs = rhs;
  node->length = length;
  return IRHandle(node);
}

IRHandle VecSubNode::make(IRHandle vec, IRHandle lhs, IRHandle rhs,
                          int length) {
  VecSubNode *node = new VecSubNode();
  node->vec = vec;
  node->lhs = lhs;
  node->rhs = rhs;
  node->length = length;
  return IRHandle(node);
}

IRHandle VecMulNode::make(IRHandle vec, IRHandle lhs, IRHandle rhs,
                          int length) {
  VecMulNode *node = new VecMulNode();
  node->vec = vec;
  node->lhs = lhs;
  node->rhs = rhs;
  node->length = length;
  return IRHandle(node);
}
IRHandle VecDivNode::make(IRHandle vec, IRHandle lhs, IRHandle rhs,
                          int length) {
  VecDivNode *node = new VecDivNode();
  node->vec = vec;
  node->lhs = lhs;
  node->rhs = rhs;
  node->length = length;
  return IRHandle(node);
}

}  // namespace polly