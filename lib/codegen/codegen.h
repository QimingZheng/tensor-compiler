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
#include <time.h>
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
  void genCode(IRHandle program, std::vector<IRHandle> &tensors);
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
};

class CodeGenCuda : public CodeGen {
 public:
};

}  // namespace polly