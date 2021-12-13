#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"
#include "expr.h"
#include "stmt.h"

namespace polly {

/// Should be Singleton.
class Program {
 private:
  ForNode *root_loop_ = nullptr;
  ForNode *current_loop_ = nullptr;

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

  bool Fuse(const std::string i, const std::string j) { return false; }

  // Divide the i loop into tiles, each of size splitFactor.
  bool Split(const std::string i, int splitFactor) {
    ForNode *outter_loop = find_loop_var(root_loop_, i);
    assert(outter_loop != nullptr);

    static_cast<VarNode *>(outter_loop->looping_var_)->name = i + "_outter";
    ForNode *inner_loop =
        new ForNode(new VarNode(i + "_inner", new IntNode(-1), new IntNode(-1),
                                new IntNode(-1)),
                    outter_loop);
    std::swap(inner_loop->body, outter_loop->body);
    outter_loop->Insert(inner_loop);

    // replace all the reference to `i` to `i_outter * splitFactor + i_inner`
    AddNode *intermediate = new AddNode(
        new MulNode(outter_loop->looping_var_, new IntNode(splitFactor)),
        inner_loop->looping_var_);

    IRMutatorVisitor mutator(outter_loop->looping_var_, intermediate);
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
};

}  // namespace polly