#pragma once

#include "common.h"

namespace polly {

enum IRNodeType {
  INT = 0,
  // FLOAT = 13,
  ADD = 1,
  SUB = 2,
  MUL = 3,
  DIV = 4,
  MOD = 5,
  VAR = 6,
  ACCESS = 7,
  ASSIGN = 8,
  TENSOR = 9,
  CONST = 10,

  FOR = 11,

  PRINT = 12,
};

class IntNode;
// class Floatnode;
class AddNode;
class SubNode;
class MulNode;
class DivNode;
class ModNode;
class AssignmentNode;
class VarNode;
class AccessNode;
class TensorNode;
class ConstNode;

class ForNode;
class PrintNode;

class IRVisitor;

typedef std::shared_ptr<AddNode> AddHandle;
typedef std::shared_ptr<SubNode> SubHandle;
typedef std::shared_ptr<MulNode> MulHandle;
typedef std::shared_ptr<DivNode> DivHandle;
typedef std::shared_ptr<ModNode> ModHandle;
typedef std::shared_ptr<VarNode> VarHandle;
typedef std::shared_ptr<IntNode> IntHandle;
typedef std::shared_ptr<TensorNode> TensorHandle;
typedef std::shared_ptr<AccessNode> AccessHandle;
typedef std::shared_ptr<AssignmentNode> AssignmentHandle;
typedef std::shared_ptr<ForNode> ForHandle;
typedef std::shared_ptr<ConstNode> ConstHandle;
typedef std::shared_ptr<PrintNode> PrintHandle;

class IRNode {
 public:
  IRNode() {}
  virtual ~IRNode() {}

  virtual IRNodeType Type() const {
    throw std::runtime_error("Type Not implemented");
  }

  virtual bool equals(const IRNode *other) {
    throw std::runtime_error("equals not implemented");
  }
};

class IRHandle {
 private:
  bool isNull() const { return ptr_ == nullptr; }

 public:
  IRHandle(std::shared_ptr<IRNode> ptr) : ptr_(ptr) {}
  IRHandle() : ptr_(nullptr) {}
  explicit IRHandle(IRNode *ptr) : ptr_(ptr) {}
  IRHandle(const IRHandle &other) : ptr_(other.ptr_) {}
  IRHandle &operator=(const IRHandle &other) { ptr_ = other.ptr_; }
  IRHandle(IRHandle &&other) : ptr_(other.ptr_) { other.ptr_ = nullptr; }
  IRHandle &operator=(IRHandle &&other) {
    ptr_ = other.ptr_;
    other.ptr_ = nullptr;
  }

  IRNode *GetRaw() const { return ptr_.get(); }

  template <typename T>
  std::shared_ptr<T> as() const {
    return std::static_pointer_cast<T>(ptr_);
  }

  bool operator!=(const IRHandle &other) { return !equals(other); }
  bool operator==(const IRHandle &other) { return equals(other); }

  mutable std::shared_ptr<IRNode> ptr_;

  bool equals(const IRHandle other) const {
    if (isNull() && other.isNull()) return true;
    if (isNull()) return false;
    if (other.isNull()) return false;
    if (other.ptr_->Type() != ptr_->Type()) return false;
    return ptr_->equals(other.GetRaw());
  }

  /// Clone this IRNode
  /// Used by the IRModule
  IRHandle clone(std::map<std::string, IRHandle> &irHandleDict);

  IRNodeType Type() const { return ptr_->Type(); }

  void accept(IRVisitor *visitor);
};

static const IRHandle NullIRHandle = IRHandle();

class AddNode : public IRNode {
 private:
  AddNode() {}

 public:
  IRHandle lhs, rhs;

  static IRHandle make(IRHandle lhs, IRHandle rhs);

  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return lhs.equals(static_cast<const AddNode *>(other)->lhs) &&
           rhs.equals(static_cast<const AddNode *>(other)->rhs);
  }

  IRNodeType Type() const override { return IRNodeType::ADD; }
};

class SubNode : public IRNode {
 private:
  SubNode() {}

 public:
  IRHandle lhs, rhs;

  static IRHandle make(IRHandle lhs, IRHandle rhs);

  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return lhs.equals(static_cast<const SubNode *>(other)->lhs) &&
           rhs.equals(static_cast<const SubNode *>(other)->rhs);
  }

  IRNodeType Type() const override { return IRNodeType::SUB; }
};

class MulNode : public IRNode {
 private:
  MulNode() {}

 public:
  IRHandle lhs, rhs;
  static IRHandle make(IRHandle lhs, IRHandle rhs);

  IRNodeType Type() const override { return IRNodeType::MUL; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return lhs.equals(static_cast<const MulNode *>(other)->lhs) &&
           rhs.equals(static_cast<const MulNode *>(other)->rhs);
  }
};

class DivNode : public IRNode {
 private:
  DivNode() {}

 public:
  IRHandle lhs, rhs;
  static IRHandle make(IRHandle lhs, IRHandle rhs);

  IRNodeType Type() const override { return IRNodeType::DIV; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return lhs.equals(static_cast<const DivNode *>(other)->lhs) &&
           rhs.equals(static_cast<const DivNode *>(other)->rhs);
  }
};

class ModNode : public IRNode {
 private:
  ModNode() {}

 public:
  IRHandle lhs, rhs;
  static IRHandle make(IRHandle lhs, IRHandle rhs);

  IRNodeType Type() const override { return IRNodeType::MOD; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return lhs.equals(static_cast<const ModNode *>(other)->lhs) &&
           rhs.equals(static_cast<const ModNode *>(other)->rhs);
  }
};

class VarNode : public IRNode {
 private:
  VarNode() {}

 public:
  std::string name;
  IRHandle min, max, increment;

  static IRHandle make(const std::string name, IRHandle min, IRHandle max,
                       IRHandle increment);

  IRNodeType Type() const override { return IRNodeType::VAR; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return name == static_cast<const VarNode *>(other)->name;
  }
};

class IntNode : public IRNode {
 private:
  IntNode() {}
  IntNode(int x) : value(x) {}

 public:
  int value;

  static IRHandle make(int x);

  IRNodeType Type() const override { return IRNodeType::INT; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return value == static_cast<const IntNode *>(other)->value;
  }
};

class TensorNode : public IRNode {
 private:
  TensorNode() {}

 public:
  std::string name;
  std::vector<int64_t> shape;

  static IRHandle make(const std::string &name, std::vector<int64_t> &shape);

  IRNodeType Type() const override { return IRNodeType::TENSOR; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return name == static_cast<const TensorNode *>(other)->name;
  }
};

class AccessNode : public IRNode {
 private:
  AccessNode() {}

 public:
  IRHandle tensor;
  std::vector<IRHandle> indices;

  static IRHandle make(IRHandle tensor, std::vector<IRHandle> indices);

  IRNodeType Type() const override { return IRNodeType::ACCESS; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    const AccessNode *tmp = static_cast<const AccessNode *>(other);
    if (indices.size() != tmp->indices.size()) return false;
    if (tensor.equals(tmp->tensor) == false) return false;
    for (int i = 0; i < indices.size(); i++) {
      if (indices[i].equals(tmp->indices[i]) == false) return false;
    }
    return true;
  }
};

class AssignmentNode : public IRNode {
 private:
  AssignmentNode() {}

 public:
  IRHandle lhs, rhs;
  static IRHandle make(IRHandle lhs, IRHandle rhs);

  IRNodeType Type() const override { return IRNodeType::ASSIGN; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    const AssignmentNode *tmp = static_cast<const AssignmentNode *>(other);
    return lhs.equals(tmp->lhs) && rhs.equals(tmp->rhs);
  }
};

class ForNode : public IRNode {
 private:
  ForNode() {}

 public:
  IRHandle looping_var_;
  std::vector<IRHandle> body;

  static IRHandle make(IRHandle looping_var);

  void Insert(IRHandle node) { body.push_back(node); }

  IRNodeType Type() const override { return IRNodeType::FOR; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    const ForNode *tmp = static_cast<const ForNode *>(other);
    if (looping_var_.equals(tmp->looping_var_) == false) return false;
    if (body.size() != tmp->body.size()) return false;
    for (int i = 0; i < body.size(); i++) {
      if (body[i].equals(tmp->body[i]) == false) return false;
    }
    return true;
  }
};

class ConstNode : public IRNode {
 private:
  ConstNode() {}

 public:
  std::string name;

  static IRHandle make(const std::string name);

  IRNodeType Type() const override { return IRNodeType::CONST; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return static_cast<const ConstNode *>(other)->name == name;
  }
};

class PrintNode : public IRNode {
 private:
  PrintNode() {}

 public:
  IRHandle print;

  static IRHandle make(IRHandle print);

  IRNodeType Type() const override { return IRNodeType::PRINT; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return print.equals(static_cast<const PrintNode *>(other)->print);
  }
};

/// TODO: add FuncNode
// class FuncNode : public IRNode {};

}  // namespace polly