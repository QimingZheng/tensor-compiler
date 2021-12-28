#include "ir.h"
#include "ir_visitor.h"

namespace polly {

IRHandle IRHandle::clone(std::map<std::string, IRHandle> &irHandleDict) {
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
      if (irHandleDict.find(as<TensorNode>()->name) != irHandleDict.end()) {
        ret = irHandleDict[as<TensorNode>()->name];
      } else {
        ret = TensorNode::make(as<TensorNode>()->name, as<TensorNode>()->shape);
        irHandleDict.insert(std::make_pair(as<TensorNode>()->name, ret));
      }
      break;
    }
    case IRNodeType::VAR: {
      if (irHandleDict.find(as<VarNode>()->name) != irHandleDict.end()) {
        ret = irHandleDict[as<VarNode>()->name];
      } else {
        ret = VarNode::make(as<VarNode>()->name,
                            as<VarNode>()->min.clone(irHandleDict),
                            as<VarNode>()->max.clone(irHandleDict),
                            as<VarNode>()->increment.clone(irHandleDict));
        irHandleDict.insert(std::make_pair(as<VarNode>()->name, ret));
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
      ret = AssignmentNode::make(as<AssignmentNode>()->lhs.clone(irHandleDict),
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
                            as<ForNode>()->looping_var_.as<VarNode>()->name) !=
          irHandleDict.end()) {
        ret = irHandleDict["For-" +
                           as<ForNode>()->looping_var_.as<VarNode>()->name];
      } else {
        ret = ForNode::make(as<ForNode>()->looping_var_.clone(irHandleDict));
        irHandleDict.insert(std::make_pair(
            "For-" + as<ForNode>()->looping_var_.as<VarNode>()->name, ret));
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
      ret = PrintNode::make(as<PrintNode>()->print.clone(irHandleDict));
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

IRHandle VarNode::make(const std::string name, IRHandle min, IRHandle max,
                       IRHandle increment) {
  VarNode *var = new VarNode();
  var->name = name;
  var->min = min;
  var->max = max;
  var->increment = increment;
  return IRHandle(var);
}

IRHandle IntNode::make(int x) {
  IntNode *node = new IntNode(x);
  return IRHandle(node);
}

IRHandle TensorNode::make(const std::string &name,
                          std::vector<int64_t> &shape) {
  TensorNode *tensor = new TensorNode();
  tensor->name = name;
  tensor->shape = shape;
  return IRHandle(tensor);
}

IRHandle AccessNode::make(IRHandle tensor, std::vector<IRHandle> indices) {
  AccessNode *node = new AccessNode;
  node->tensor = tensor;
  node->indices = indices;
  return IRHandle(node);
}

IRHandle AssignmentNode::make(IRHandle lhs, IRHandle rhs) {
  AssignmentNode *node = new AssignmentNode();
  node->lhs = lhs;
  node->rhs = rhs;
  return IRHandle(node);
}

IRHandle ForNode::make(IRHandle looping_var) {
  ForNode *node = new ForNode();
  node->looping_var_ = looping_var;
  return IRHandle(node);
}

IRHandle ConstNode::make(std::string name) {
  ConstNode *node = new ConstNode();
  node->name = name;
  return IRHandle(node);
}

IRHandle PrintNode::make(IRHandle print) {
  PrintNode *node = new PrintNode();
  node->print = print;
  return IRHandle(node);
}

void IRHandle::accept(IRVisitor *visitor) { visitor->visit(*this); }

}  // namespace polly