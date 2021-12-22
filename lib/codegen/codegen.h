#pragma once

#include "common.h"
#include "ir/ir.h"
#include "ir/ir_visitor.h"

namespace polly {

class CodeGen : public IRVisitor {
 public:
};

const std::string C_Heaader = R"(
#include <stdlib.h>
#include <stdio.h>
)";

class CodeGenC : public IRVisitor {
  std::string getIndent() {
    std::string ret = "";
    for (int i = 0; i < indent; i++) {
      ret += '\t';
    }
    return ret;
  }
  std::ostream &oss;
  int indent = 1;

 public:
  CodeGenC(std::ostream &os) : oss(os) {}
  void genCode(ForNode *program, std::vector<TensorNode *> &tensors);
  void visitInt(IntNode *int_expr) override;
  void visitAdd(AddNode *add) override;
  void visitMul(MulNode *mul) override;
  void visitVar(VarNode *var) override;
  void visitAccess(AccessNode *access) override;
  void visitAssign(AssignmentNode *assign) override;
  void visitTensor(TensorNode *tensor) override;
  void visitFor(ForNode *loop) override;
};

class CodeGenCuda : public CodeGen {
 public:
};

}  // namespace polly