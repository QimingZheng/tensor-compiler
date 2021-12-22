#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "ir/ir_check_pass.h"
#include "expr.h"
#include "stmt.h"
#include "codegen/codegen.h"

namespace polly {

/// Should be Singleton.
class Program {
 private:
  std::vector<IRHandle> tensors;

  IRHandle root_loop_;
  IRHandle current_loop_;

  /// Find the for-loop that uses var `loop_var_name` as its looping var.
  IRHandle find_loop_var(IRHandle cur, const std::string loop_var_name) {
    ForHandle curFor = cur.as<ForNode>();
    if (curFor == nullptr) return NullIRHandle;
    if (curFor->looping_var_.as<VarNode>()->name == loop_var_name) {
      return cur;
    }
    for (int i = 0; i < curFor->body.size(); i++) {
      if (curFor->body[i].Type() == IRNodeType::FOR) {
        IRHandle ret = find_loop_var(curFor->body[i], loop_var_name);
        if (ret != NullIRHandle) return ret;
      }
    }
    return NullIRHandle;
  }

  int isNestedLoop(IRHandle outter, IRHandle inner) {
    ForHandle outterFor = outter.as<ForNode>();
    ForHandle innerFor = inner.as<ForNode>();
    if (outterFor == nullptr) return false;
    if (innerFor == nullptr) return false;
    for (int i = 0; i < outterFor->body.size(); i++) {
      if (outterFor->body[i].Type() == IRNodeType::FOR) {
        if (outterFor->body[i].equals(inner)) return i;
      }
    }
    return -1;
  }

 protected:
  static Program *singleton_;
  std::string value_;

 public:
  Program() {
    if (singleton_ != nullptr)
      throw std::runtime_error("A program is already created\n");
    singleton_ = this;
  }

  ~Program() { singleton_ = nullptr; }
  Program(Program &other) = delete;
  void operator=(const Program &) = delete;

  static Program *GetInstance();

  bool EnterLoop(const Variable *var) {
    if (root_loop_ == NullIRHandle) {
      root_loop_ = ForNode::make(var->GetIRHandle());
      current_loop_ = root_loop_;
    } else {
      IRHandle loop = ForNode::make(var->GetIRHandle(), current_loop_);
      current_loop_.as<ForNode>()->Insert(loop);
      current_loop_ = loop;
    }
    return false;
  }

  bool AddStmt(Stmt *stmt) {
    IRPrinterVisitor visitor;
    current_loop_.as<ForNode>()->Insert(stmt->GetIRHandle());
    return false;
  }

  bool ExitLoop(const Variable *var) {
    current_loop_ = current_loop_.as<ForNode>()->parent_loop_;
    return false;
  }

  bool Reorder(const std::string i, const std::string j) {
    IRHandle i_loop = find_loop_var(root_loop_, i);
    IRHandle j_loop = find_loop_var(root_loop_, j);
    assert(i_loop != NullIRHandle);
    assert(j_loop != NullIRHandle);
    std::swap(i_loop.as<ForNode>()->looping_var_,
              j_loop.as<ForNode>()->looping_var_);
    return true;
  }

  bool Fuse(const std::string i, const std::string j) {
    IRHandle outter_loop = find_loop_var(root_loop_, i);
    IRHandle inner_loop = find_loop_var(root_loop_, j);
    assert(outter_loop != NullIRHandle);
    assert(inner_loop != NullIRHandle);
    int inner_loop_pos = isNestedLoop(outter_loop, inner_loop);
    assert(inner_loop_pos >= 0);

    VarHandle outter = outter_loop.as<ForNode>()->looping_var_.as<VarNode>();
    VarHandle inner = inner_loop.as<ForNode>()->looping_var_.as<VarNode>();
    IRHandle outter_lower = outter->min, outter_upper = outter->max,
             outter_inc = outter->increment;
    IRHandle inner_lower = inner->min, inner_upper = inner->max,
             inner_inc = inner->increment;

    outter->name += inner->name;

    // get rid off the inner loop, now we only have the fused loop
    for (int i = 0; i < inner_loop.as<ForNode>()->body.size(); i++) {
      outter_loop.as<ForNode>()->body.insert(
          outter_loop.as<ForNode>()->body.begin() + inner_loop_pos++,
          inner_loop.as<ForNode>()->body[i]);
    }
    outter_loop.as<ForNode>()->body.erase(
        outter_loop.as<ForNode>()->body.begin() + inner_loop_pos);

    outter->min = IntNode::make(0);
    outter->max = MulNode::make(SubNode::make(outter_upper, outter_lower),
                                SubNode::make(inner_upper, inner_lower));
    outter->increment = MulNode::make(outter_inc, inner_inc);

    IRHandle common =
        MulNode::make(SubNode::make(inner_upper, outter_upper), outter_inc);
    IRMutatorVisitor outter_mutator(
        outter_loop.as<ForNode>()->looping_var_,
        AddNode::make(
            MulNode::make(
                outter_inc,
                DivNode::make(outter_loop.as<ForNode>()->looping_var_, common)),
            outter_lower));
    outter_mutator.visit(root_loop_);

    IRMutatorVisitor inner_mutator(
        inner_loop.as<ForNode>()->looping_var_,
        AddNode::make(
            MulNode::make(
                inner_inc,
                ModNode::make(outter_loop.as<ForNode>()->looping_var_, common)),
            inner_lower));
    inner_mutator.visit(root_loop_);

    return true;
  }

  // Divide the i loop into `tiles` tiles.
  // each of size splitFactor.
  // bool Split(const std::string i, int splitFactor) {
  bool Split(const std::string i, Expr tiles) {
    IRHandle tileNode = tiles.GetIRHandle();
    IRHandle outter_loop = find_loop_var(root_loop_, i);
    assert(outter_loop != NullIRHandle);

    VarHandle originLoopVar =
        outter_loop.as<ForNode>()->looping_var_.as<VarNode>();

    originLoopVar->name = i + "_outter";
    IRHandle stride =
        DivNode::make(SubNode::make(originLoopVar->max, originLoopVar->min),
                      MulNode::make(tileNode, originLoopVar->increment));
    IRHandle inner_loop = ForNode::make(
        VarNode::make(i + "_inner", IntNode::make(0), stride, IntNode::make(1)),
        outter_loop);

    originLoopVar->min = IntNode::make(0);
    originLoopVar->max = tileNode;
    originLoopVar->increment = IntNode::make(1);

    std::swap(inner_loop.as<ForNode>()->body, outter_loop.as<ForNode>()->body);
    outter_loop.as<ForNode>()->Insert(inner_loop);

    // replace all the reference to `i` to `i_outter * tiles + i_inner`
    // AddNode *intermediate = new AddNode(
    //     new MulNode(outter_loop->looping_var_, new IntNode(tiles)),
    //     inner_loop->looping_var_);
    IRHandle intermediate = AddNode::make(
        MulNode::make(outter_loop.as<ForNode>()->looping_var_, stride),
        inner_loop.as<ForNode>()->looping_var_);

    IRMutatorVisitor mutator(outter_loop.as<ForNode>()->looping_var_,
                             intermediate);
    /// Root-Loop is a ForNode, so it cannot be replaced in any cases, so we
    /// always replace from the descendants of the Root-For-Loop-Node.

    mutator.visit(root_loop_);

    return true;
  }

  bool Parallel(const std::string i) { return false; }

  bool Vectorize(const std::string i, int vectorLength) {
    Split(i, vectorLength);
    return true;
  }

  void IRGen() {
    IRPrinterVisitor visitor;
    std::cout << "====================\n";
    visitor.visit(root_loop_);
    std::cout << "====================\n";
  }

  bool IsAffineProgram() {
    IRCheckAffinePass check;
    return check.checkFor(root_loop_.as<ForNode>());
  }

  void GenerateC() {
    CodeGenC codegen(std::cout);
    codegen.genCode(root_loop_, tensors);
    // std::cout << std::string(codegen.oss);
  }

  void DeclareTensor(Tensor *tensor) {
    tensors.push_back(static_cast<IRHandle>(tensor->GetIRHandle()));
  }
};

}  // namespace polly