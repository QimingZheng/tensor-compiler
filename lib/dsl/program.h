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
  std::vector<TensorNode *> tensors;

  ForNode *root_loop_ = nullptr;
  ForNode *current_loop_ = nullptr;

  /// Find the for-loop that uses var `loop_var_name` as its looping var.
  ForNode *find_loop_var(ForNode *cur, const std::string loop_var_name) {
    if (cur == nullptr) return nullptr;
    if (static_cast<VarNode *>(cur->looping_var_)->name == loop_var_name) {
      return cur;
    }
    for (int i = 0; i < cur->body.size(); i++) {
      if (cur->body[i]->Type() == IRNodeType::FOR) {
        ForNode *ret =
            find_loop_var(static_cast<ForNode *>(cur->body[i]), loop_var_name);
        if (ret != nullptr) return ret;
      }
    }
    return nullptr;
  }

  int isNestedLoop(ForNode *outter, ForNode *inner) {
    if (outter == nullptr) return false;
    if (inner == nullptr) return false;
    for (int i = 0; i < outter->body.size(); i++) {
      if (outter->body[i]->Type() == IRNodeType::FOR) {
        if (outter->body[i]->equals(inner)) return i;
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
    if (root_loop_ == nullptr) {
      root_loop_ = new ForNode(var->GetIRNode());
      current_loop_ = root_loop_;
    } else {
      ForNode *loop = new ForNode(var->GetIRNode(), current_loop_);
      current_loop_->Insert(loop);
      current_loop_ = loop;
    }
    return false;
  }

  bool AddStmt(Stmt *stmt) {
    IRPrinterVisitor visitor;
    current_loop_->Insert(stmt->GetIRNode());
    return false;
  }

  bool ExitLoop(const Variable *var) {
    current_loop_ = current_loop_->parent_loop_;
    return false;
  }

  bool Reorder(const std::string i, const std::string j) {
    ForNode *i_loop = find_loop_var(root_loop_, i);
    ForNode *j_loop = find_loop_var(root_loop_, j);
    assert(i_loop != nullptr);
    assert(j_loop != nullptr);
    std::swap(i_loop->looping_var_, j_loop->looping_var_);
    return true;
  }

  bool Fuse(const std::string i, const std::string j) {
    ForNode *outter_loop = find_loop_var(root_loop_, i);
    ForNode *inner_loop = find_loop_var(root_loop_, j);
    assert(outter_loop != nullptr);
    assert(inner_loop != nullptr);
    int inner_loop_pos = isNestedLoop(outter_loop, inner_loop);
    assert(inner_loop_pos >= 0);

    VarNode *outter = static_cast<VarNode *>(outter_loop->looping_var_);
    VarNode *inner = static_cast<VarNode *>(inner_loop->looping_var_);
    IRNode *outter_lower = outter->min, *outter_upper = outter->max,
           *outter_inc = outter->increment;
    IRNode *inner_lower = inner->min, *inner_upper = inner->max,
           *inner_inc = inner->increment;

    outter->name += inner->name;

    // get rid off the inner loop, now we only have the fused loop
    for (int i = 0; i < inner_loop->body.size(); i++) {
      outter_loop->body.insert(outter_loop->body.begin() + inner_loop_pos++,
                               inner_loop->body[i]);
    }
    outter_loop->body.erase(outter_loop->body.begin() + inner_loop_pos);

    outter->min = new IntNode(0);
    outter->max = new MulNode(new SubNode(outter_upper, outter_lower),
                              new SubNode(inner_upper, inner_lower));
    outter->increment = new MulNode(outter_inc, inner_inc);

    MulNode *common =
        new MulNode(new SubNode(inner_upper, outter_upper), outter_inc);
    IRMutatorVisitor outter_mutator(
        outter,
        new AddNode(new MulNode(outter_inc, new DivNode(outter, common)),
                    outter_lower));
    outter_mutator.visit(root_loop_);
    IRMutatorVisitor inner_mutator(
        inner, new AddNode(new MulNode(inner_inc, new ModNode(outter, common)),
                           inner_lower));
    inner_mutator.visit(root_loop_);

    return true;
  }

  // Divide the i loop into `tiles` tiles.
  // each of size splitFactor.
  // bool Split(const std::string i, int splitFactor) {
  bool Split(const std::string i, Expr tiles) {
    IRNode *tileNode = tiles.expr_node_;
    ForNode *outter_loop = find_loop_var(root_loop_, i);
    assert(outter_loop != nullptr);

    VarNode *originLoopVar = static_cast<VarNode *>(outter_loop->looping_var_);

    originLoopVar->name = i + "_outter";
    DivNode *stride =
        new DivNode(new SubNode(originLoopVar->max, originLoopVar->min),
                    new MulNode(tileNode, originLoopVar->increment));
    ForNode *inner_loop = new ForNode(
        new VarNode(i + "_inner", new IntNode(0), stride, new IntNode(1)),
        outter_loop);

    originLoopVar->min = new IntNode(0);
    originLoopVar->max = tileNode;
    originLoopVar->increment = new IntNode(1);

    std::swap(inner_loop->body, outter_loop->body);
    outter_loop->Insert(inner_loop);

    // replace all the reference to `i` to `i_outter * tiles + i_inner`
    // AddNode *intermediate = new AddNode(
    //     new MulNode(outter_loop->looping_var_, new IntNode(tiles)),
    //     inner_loop->looping_var_);
    AddNode *intermediate = new AddNode(new MulNode(originLoopVar, stride),
                                        inner_loop->looping_var_);

    IRMutatorVisitor mutator(originLoopVar, intermediate);
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
    return check.checkFor(root_loop_);
  }

  void GenerateC() {
    CodeGenC codegen(std::cout);
    codegen.genCode(root_loop_, tensors);
    // std::cout << std::string(codegen.oss);
  }

  void DeclareTensor(Tensor *tensor) {
    tensors.push_back(static_cast<TensorNode *>(tensor->GetIRNode()));
  }
};

}  // namespace polly