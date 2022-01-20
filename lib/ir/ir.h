/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:32:50
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-19 21:49:23
 * @CopyRight: Qiming Zheng
 */
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
  FUNC = 13,
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
class FuncNode;

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
typedef std::shared_ptr<FuncNode> FuncHandle;

/// IRNodeKey is used to identify a certain IR-Node.
typedef std::string IRNodeKey;

class IRNodeKeyGen {
  IRNodeKeyGen() {}
  int id = 0;

 protected:
  static IRNodeKeyGen *generator;

 public:
  static IRNodeKeyGen *GetInstance();
  std::string yield() { return std::to_string(id++); }
};

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

  bool operator!=(const IRHandle &other) const { return !equals(other); }
  bool operator==(const IRHandle &other) const { return equals(other); }

  mutable std::shared_ptr<IRNode> ptr_;

  bool equals(const IRHandle other) const {
    if (isNull() && other.isNull()) return true;
    if (isNull()) return false;
    if (other.isNull()) return false;
    if (other.ptr_->Type() != ptr_->Type()) return false;
    return ptr_->equals(other.GetRaw());
  }

  /// Clone this IRNode Recursively
  /// Used by the IRModule
  IRHandle clone(std::map<std::string, IRHandle> &irHandleDict);

  IRNodeType Type() const { return ptr_->Type(); }

  void accept(IRVisitor *visitor);
};

// hash function for IRHandles
class IRHandleHash {
 public:
  // id is returned as hash function
  size_t operator()(const IRHandle &t) const { return t.Type(); }
};

static const IRHandle NullIRHandle = IRHandle();

class BinaryNode : public IRNode {
 public:
  IRHandle lhs, rhs;

  template <typename T>
  bool equals(const IRNode *other) {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return lhs.equals(static_cast<const T *>(other)->lhs) &&
           rhs.equals(static_cast<const T *>(other)->rhs);
  }
};

class AddNode : public BinaryNode {
 private:
  AddNode() {}

 public:
  // IRHandle lhs, rhs;

  static IRHandle make(IRHandle lhs, IRHandle rhs);

  bool equals(const IRNode *other) override {
    return BinaryNode::equals<AddNode>(other);
  }

  IRNodeType Type() const override { return IRNodeType::ADD; }
};

class SubNode : public BinaryNode {
 private:
  SubNode() {}

 public:
  // IRHandle lhs, rhs;

  static IRHandle make(IRHandle lhs, IRHandle rhs);

  bool equals(const IRNode *other) override {
    return BinaryNode::equals<SubNode>(other);
  }

  IRNodeType Type() const override { return IRNodeType::SUB; }
};

class MulNode : public BinaryNode {
 private:
  MulNode() {}

 public:
  // IRHandle lhs, rhs;
  static IRHandle make(IRHandle lhs, IRHandle rhs);

  IRNodeType Type() const override { return IRNodeType::MUL; }
  bool equals(const IRNode *other) override {
    return BinaryNode::equals<MulNode>(other);
  }
};

class DivNode : public BinaryNode {
 private:
  DivNode() {}

 public:
  // IRHandle lhs, rhs;
  static IRHandle make(IRHandle lhs, IRHandle rhs);

  IRNodeType Type() const override { return IRNodeType::DIV; }
  bool equals(const IRNode *other) override {
    return BinaryNode::equals<DivNode>(other);
  }
};

class ModNode : public BinaryNode {
 private:
  ModNode() {}

 public:
  // IRHandle lhs, rhs;
  static IRHandle make(IRHandle lhs, IRHandle rhs);

  IRNodeType Type() const override { return IRNodeType::MOD; }

  bool equals(const IRNode *other) override {
    return BinaryNode::equals<ModNode>(other);
  }
};

class VarNode : public IRNode {
 private:
  VarNode() {}

 public:
  IRNodeKey id;
  IRHandle min, max, increment;

  static IRHandle make(const IRNodeKey id, IRHandle min, IRHandle max,
                       IRHandle increment);

  IRNodeType Type() const override { return IRNodeType::VAR; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return id == static_cast<const VarNode *>(other)->id;
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
  IRNodeKey id;
  std::vector<int64_t> shape;

  static IRHandle make(const IRNodeKey id, std::vector<int64_t> &shape);

  IRNodeType Type() const override { return IRNodeType::TENSOR; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return id == static_cast<const TensorNode *>(other)->id;
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
  IRNodeKey id;
  IRHandle lhs, rhs;
  static IRHandle make(IRNodeKey id, IRHandle lhs, IRHandle rhs);

  IRNodeType Type() const override { return IRNodeType::ASSIGN; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    const AssignmentNode *tmp = static_cast<const AssignmentNode *>(other);

    return lhs.equals(tmp->lhs) && rhs.equals(tmp->rhs) && id == tmp->id;
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
  IRNodeKey id;
  IRHandle print;

  static IRHandle make(IRNodeKey id, IRHandle print);

  IRNodeType Type() const override { return IRNodeType::PRINT; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    return print.equals(static_cast<const PrintNode *>(other)->print);
  }
};

class FuncNode : public IRNode {
 private:
  FuncNode() {}

 public:
  std::vector<IRHandle> body;

  static IRHandle make(std::vector<IRHandle> body);
  IRNodeType Type() const override { return IRNodeType::FUNC; }
  bool equals(const IRNode *other) override {
    if (other == nullptr) return false;
    if (Type() != other->Type()) return false;
    if (body.size() != static_cast<const FuncNode *>(other)->body.size())
      return false;
    for (int i = 0; i < body.size(); i++) {
      if (!body[i].equals(static_cast<const FuncNode *>(other)->body[i]))
        return false;
    }
    return true;
  }
};

// TODO: Complete the unary node.
class UnaryNode : public IRNode {
 public:
  IRHandle node;
};

// TODO: Complete the negate node.
class NegateNode : public UnaryNode {
 public:
};

// sin(x)
class SinNode : public UnaryNode {};
// cos(x)
class CosNode : public UnaryNode {};
// tan(x)
class TanNode : public UnaryNode {};
// exp(x)
class ExpNode : public UnaryNode {};
// abs(x)
class AbsNode : public UnaryNode {};
// sign(x)
class SignNode : public UnaryNode {};
// sqrt(x)
class SqrtNode : public UnaryNode {};

// min(a, b)
class MinNode : public BinaryNode {};
// max(a, b)
class MaxNode : public BinaryNode {};
// >=
class GeNode : public BinaryNode {};
// <=
class LeNode : public BinaryNode {};
// >
class GtNode : public BinaryNode {};
// <
class LtNode : public BinaryNode {};
// ==
class EqNode : public BinaryNode {};
// !=
class NeqNode : public BinaryNode {};

class SIMDLoad : public UnaryNode {};

class SIMDStore : public UnaryNode {};

class SIMDAdd : public BinaryNode {};
class SIMDSub : public BinaryNode {};
class SIMDMul : public BinaryNode {};
class SIMDDiv : public BinaryNode {};

}  // namespace polly