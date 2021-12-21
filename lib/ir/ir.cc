#include "ir.h"
#include "ir_visitor.h"

namespace polly {

void AccessNode::accept(IRVisitor *visitor) { visitor->visit(this); }

void AddNode::accept(IRVisitor *visitor) { visitor->visit(this); }

void MulNode::accept(IRVisitor *visitor) { visitor->visit(this); }

void SubNode::accept(IRVisitor *visitor) { visitor->visit(this); }

void DivNode::accept(IRVisitor *visitor) { visitor->visit(this); }

void ModNode::accept(IRVisitor *visitor) { visitor->visit(this); }

void VarNode::accept(IRVisitor *visitor) { visitor->visit(this); }

void ConstNode::accept(IRVisitor *visitor) { visitor->visit(this); }

void IntNode::accept(IRVisitor *visitor) { visitor->visit(this); }

void TensorNode::accept(IRVisitor *visitor) { visitor->visit(this); }

void AssignmentNode::accept(IRVisitor *visitor) { visitor->visit(this); }

void ForNode::accept(IRVisitor *visitor) { visitor->visit(this); }

}  // namespace polly