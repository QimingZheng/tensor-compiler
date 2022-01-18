/*
 * @Description: Polly: A DSL compiler for Tensor Program 
 * @Author: Qiming Zheng 
 * @Date: 2022-01-18 20:32:29 
 * @Last Modified by:   Qiming Zheng 
 * @Last Modified time: 2022-01-18 20:32:29 
 * @CopyRight: Qiming Zheng 
 */
#pragma once

#include "common.h"

namespace polly {

class Expr;

class Stmt {
 public:
  mutable IRHandle handle_;
  IRHandle GetIRHandle() const { return handle_; }
};

/// All Assignments will be registered into the ast tree.
class Assignment : public Stmt {
 public:
  Assignment(const Expr lhs, const Expr &rhs);
};

class Print : public Stmt {
 public:
  Print(const Expr print);
};

}  // namespace polly