#include "affine_check.h"

namespace polly {

void AffineCheck::visitInt(IntHandle int_expr) {
  // PASS
  isAffine = true;
}
void AffineCheck::visitFloat(FloatHandle float_expr) {
  // PASS
  isAffine = false;
}
void AffineCheck::visitAdd(AddHandle add) {
  if (!firstTimeEntering) {
    auto ret = IsAffineIRHandle::runPass(
        std::shared_ptr<IsAffineIRHandle::Arg>(new IsAffineIRHandle::Arg(add)));
    isAffine = PassRet::as<IsAffineIRHandle::Ret>(ret)->isAffine;
  } else {
    add->lhs.accept(this);
    if (!isAffine) return;
    add->rhs.accept(this);
    if (!isAffine) return;
  }
}
void AffineCheck::visitSub(SubHandle sub) {
  if (!firstTimeEntering) {
    auto ret = IsAffineIRHandle::runPass(
        std::shared_ptr<IsAffineIRHandle::Arg>(new IsAffineIRHandle::Arg(sub)));
    isAffine = PassRet::as<IsAffineIRHandle::Ret>(ret)->isAffine;
  } else {
    sub->lhs.accept(this);
    if (!isAffine) return;
    sub->rhs.accept(this);
    if (!isAffine) return;
  }
}
void AffineCheck::visitMul(MulHandle mul) {
  if (!firstTimeEntering) {
    auto ret = IsAffineIRHandle::runPass(
        std::shared_ptr<IsAffineIRHandle::Arg>(new IsAffineIRHandle::Arg(mul)));
    isAffine = PassRet::as<IsAffineIRHandle::Ret>(ret)->isAffine;
  } else {
    mul->lhs.accept(this);
    if (!isAffine) return;
    mul->rhs.accept(this);
    if (!isAffine) return;
  }
}
void AffineCheck::visitDiv(DivHandle div) {
  if (!firstTimeEntering) {
    auto ret = IsAffineIRHandle::runPass(
        std::shared_ptr<IsAffineIRHandle::Arg>(new IsAffineIRHandle::Arg(div)));
    isAffine = PassRet::as<IsAffineIRHandle::Ret>(ret)->isAffine;
  } else {
    div->lhs.accept(this);
    if (!isAffine) return;
    div->rhs.accept(this);
    if (!isAffine) return;
  }
}
void AffineCheck::visitMod(ModHandle mod) {
  if (!firstTimeEntering) {
    auto ret = IsAffineIRHandle::runPass(
        std::shared_ptr<IsAffineIRHandle::Arg>(new IsAffineIRHandle::Arg(mod)));
    isAffine = PassRet::as<IsAffineIRHandle::Ret>(ret)->isAffine;
  } else {
    mod->lhs.accept(this);
    if (!isAffine) return;
    mod->rhs.accept(this);
    if (!isAffine) return;
  }
}
void AffineCheck::visitVar(VarHandle var) {
  auto ret = IsAffineIRHandle::runPass(
      std::shared_ptr<IsAffineIRHandle::Arg>(new IsAffineIRHandle::Arg(var)));
  isAffine = PassRet::as<IsAffineIRHandle::Ret>(ret)->isAffine;
}
void AffineCheck::visitAccess(AccessHandle access) {
  if (firstTimeEntering) {
    firstTimeEntering = false;
    for (int i = 0; i < access->indices.size(); i++) {
      this->visit(access->indices[i]);
      if (!isAffine) break;
    }
    firstTimeEntering = true;
  } else {
    isAffine = false;
    return;
  }
}
void AffineCheck::visitAssign(AssignmentHandle assign) {
  firstTimeEntering = true;
  this->visit(assign->lhs);
  firstTimeEntering = false;
  if (!isAffine) return;
  firstTimeEntering = true;
  this->visit(assign->rhs);
  firstTimeEntering = false;
  if (!isAffine) return;
}
void AffineCheck::visitTensor(TensorHandle tensor) {
  /// Pass
  isAffine = true;
}
void AffineCheck::visitVal(ValHandle val) {
  /// Pass
  if (!firstTimeEntering) {
    isAffine = false;
  }
}

void AffineCheck::visitDecl(DeclHandle decl) {
  /// Pass
  isAffine = true;
}

void AffineCheck::visitFor(ForHandle loop) {
  {
    auto ret = IsAffineIRHandle::runPass(std::shared_ptr<IsAffineIRHandle::Arg>(
        new IsAffineIRHandle::Arg(loop->looping_var_.as<VarNode>()->min)));
    isAffine = PassRet::as<IsAffineIRHandle::Ret>(ret)->isAffine;
    if (!isAffine) return;
    ret = IsAffineIRHandle::runPass(std::shared_ptr<IsAffineIRHandle::Arg>(
        new IsAffineIRHandle::Arg(loop->looping_var_.as<VarNode>()->max)));
    isAffine = PassRet::as<IsAffineIRHandle::Ret>(ret)->isAffine;
    if (!isAffine) return;
  }
  {
    auto ret = IsConstantIRHandle::runPass(
        std::shared_ptr<IsConstantIRHandle::Arg>(new IsConstantIRHandle::Arg(
            loop->looping_var_.as<VarNode>()->increment)));
    isAffine = PassRet::as<IsConstantIRHandle::Ret>(ret)->isConstant;
    if (!isAffine) return;
  }
  for (int i = 0; i < loop->body.size(); i++) {
    loop->body[i].accept(this);
    if (!isAffine) return;
  }
  isAffine = true;
}

void AffineCheck::visitConst(ConstHandle con) { isAffine = true; }

void AffineCheck::visitPrint(PrintHandle print) {
  auto ret = IsAffineIRHandle::runPass(std::shared_ptr<IsAffineIRHandle::Arg>(
      new IsAffineIRHandle::Arg(print->print)));
  isAffine = PassRet::as<IsAffineIRHandle::Ret>(ret)->isAffine;
}

void AffineCheck::visitFunc(FuncHandle func) {
  for (int i = 0; i < func->body.size(); i++) {
    func->body[i].accept(this);
    if (!isAffine) return;
  }
  isAffine = true;
}

void AffineCheck::visitMin(MinHandle min) {
  if (!firstTimeEntering) {
    auto ret = IsAffineIRHandle::runPass(
        std::shared_ptr<IsAffineIRHandle::Arg>(new IsAffineIRHandle::Arg(min)));
    isAffine = PassRet::as<IsAffineIRHandle::Ret>(ret)->isAffine;
  } else {
    min->lhs.accept(this);
    if (!isAffine) return;
    min->rhs.accept(this);
    if (!isAffine) return;
  }
}

void AffineCheck::visitMax(MaxHandle max) {
  if (!firstTimeEntering) {
    auto ret = IsAffineIRHandle::runPass(
        std::shared_ptr<IsAffineIRHandle::Arg>(new IsAffineIRHandle::Arg(max)));
    isAffine = PassRet::as<IsAffineIRHandle::Ret>(ret)->isAffine;
  } else {
    max->lhs.accept(this);
    if (!isAffine) return;
    max->rhs.accept(this);
    if (!isAffine) return;
  }
}

}  // namespace polly