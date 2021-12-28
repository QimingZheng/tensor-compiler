#pragma once

#include "ir/ir.h"
#include "ir/ir_workspace.h"

namespace polly {

/// Jitter Module that executes a program.
class JitModule : public IRVisitor {
 public:
  JitModule(const IRWorkSpace &workspace) : workspace_(workspace) {}
  ~JitModule() {
    for (auto tensor : tensors_) {
      delete[] tensor.second;
    }
  }
  void execute() { visit(workspace_.GetRoot()); }

  void visitInt(IntHandle int_expr) override;
  void visitAdd(AddHandle add) override;
  void visitSub(SubHandle sub) override;
  void visitMul(MulHandle mul) override;
  void visitDiv(DivHandle div) override;
  void visitMod(ModHandle mod) override;
  void visitVar(VarHandle var) override;
  void visitAccess(AccessHandle access) override;
  void visitAssign(AssignmentHandle assign) override;
  void visitTensor(TensorHandle tensor) override;
  void visitFor(ForHandle loop) override;
  void visitConst(ConstHandle con) override;
  void visitPrint(PrintHandle print) override;

 private:
  union value {
    int int_value;
    float float_value;
  };
  enum value_type {
    INT,
    FLOAT,
  };
  float *tensor_ptr;
  value v;
  value_type t;
  IRWorkSpace workspace_;
  std::map<std::string, value> symbols_;
  std::map<std::string, float *> tensors_;
  std::map<std::string, std::vector<int>> tensor_shapes_;
};

}  // namespace polly
