#pragma once

#include "common.h"

namespace polly {

enum IRNodeType {
  INT = 0,
  ADD = 1,
  MUL = 2,
  VAR = 3,
  ACCESS = 4,
  ASSIGN = 5,
  TENSOR = 6,

  ////
  FOR = 8,
};

class IntNode;
class AddNode;
class MulNode;
class AssignmentNode;
class VarNode;
class AccessNode;
class TensorNode;

////
class ForNode;

class IRVisitor;

class IRNode {
 public:
  virtual void accept(IRVisitor *visitor) {
    throw std::runtime_error("Accept Not implemented");
  }

  virtual IRNodeType Type() const {
    throw std::runtime_error("Type Not implemented");
  }

  virtual bool equals(const IRNode *other) {
    throw std::runtime_error("equals not implemented");
  }
};

class AddNode : public IRNode {
 public:
  IRNode *lhs, *rhs;
  AddNode() = delete;
  AddNode(IRNode *lhs, IRNode *rhs) : lhs(lhs), rhs(rhs) {}

  void accept(IRVisitor *visitor) override;
  IRNodeType Type() const override { return IRNodeType::ADD; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return lhs->equals(static_cast<const AddNode *>(other)->lhs) &&
           rhs->equals(static_cast<const AddNode *>(other)->rhs);
  }
};

class MulNode : public IRNode {
 public:
  IRNode *lhs, *rhs;
  MulNode() = delete;
  MulNode(IRNode *lhs, IRNode *rhs) : lhs(lhs), rhs(rhs) {}

  void accept(IRVisitor *visitor) override;
  IRNodeType Type() const override { return IRNodeType::MUL; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return lhs->equals(static_cast<const MulNode *>(other)->lhs) &&
           rhs->equals(static_cast<const MulNode *>(other)->rhs);
  }
};

class VarNode : public IRNode {
 public:
  std::string name;
  IRNode *min, *max, *increment;
  VarNode() = delete;
  VarNode(const std::string name, IRNode *min, IRNode *max, IRNode *increment)
      : name(name), min(min), max(max), increment(increment) {}
  void accept(IRVisitor *visitor) override;

  IRNodeType Type() const override { return IRNodeType::VAR; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return name == static_cast<const VarNode *>(other)->name;
  }
};

class IntNode : public IRNode {
 public:
  int value;
  IntNode() = delete;
  IntNode(int x) : value(x) {}
  void accept(IRVisitor *visitor) override;

  IRNodeType Type() const override { return IRNodeType::INT; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return value == static_cast<const IntNode *>(other)->value;
  }
};

class TensorNode : public IRNode {
 public:
  std::string name;
  TensorNode() = delete;
  TensorNode(const std::string &name) : name(name) {}

  void accept(IRVisitor *visitor) override;
  IRNodeType Type() const override { return IRNodeType::TENSOR; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return name == static_cast<const TensorNode *>(other)->name;
  }
};

class AccessNode : public IRNode {
 public:
  AccessNode() = delete;
  IRNode *tensor;
  std::vector<IRNode *> indices;
  AccessNode(IRNode *tensor, std::vector<IRNode *> indices)
      : tensor(tensor), indices(indices) {}

  void accept(IRVisitor *visitor) override;
  IRNodeType Type() const override { return IRNodeType::ACCESS; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    const AccessNode *tmp = static_cast<const AccessNode *>(other);
    if (indices.size() != tmp->indices.size()) return false;
    if (tensor->equals(tmp->tensor) == false) return false;
    for (int i = 0; i < indices.size(); i++) {
      if (indices[i]->equals(tmp->indices[i]) == false) return false;
    }
    return true;
  }
};

class AssignmentNode : public IRNode {
 public:
  AssignmentNode() = delete;
  IRNode *lhs, *rhs;
  AssignmentNode(IRNode *lhs, IRNode *rhs) : lhs(lhs), rhs(rhs) {}

  void accept(IRVisitor *visitor) override;

  IRNodeType Type() const override { return IRNodeType::ASSIGN; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    const AssignmentNode *tmp = static_cast<const AssignmentNode *>(other);
    return lhs->equals(tmp->lhs) && rhs->equals(tmp->rhs);
  }
};

class ForNode : public IRNode {
 public:
  IRNode *looping_var_;
  ForNode *parent_loop_;
  std::vector<IRNode *> body;

  ForNode(IRNode *looping_var, ForNode *parent_loop = nullptr)
      : looping_var_(looping_var), parent_loop_(parent_loop) {}

  void Insert(IRNode *node) { body.push_back(node); }

  void accept(IRVisitor *visitor) override;
  IRNodeType Type() const override { return IRNodeType::FOR; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    const ForNode *tmp = static_cast<const ForNode *>(other);
    if (looping_var_->equals(tmp->looping_var_) == false) return false;
    if (body.size() != tmp->body.size()) return false;
    for (int i = 0; i < body.size(); i++) {
      if (body[i]->equals(tmp->body[i]) == false) return false;
    }
    return true;
  }
};

}  // namespace polly