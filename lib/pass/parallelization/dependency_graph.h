/*
 * @Description: Polly: A DSL compiler for Tensor Program 
 * @Author: Qiming Zheng 
 * @Date: 2022-01-18 20:31:15 
 * @Last Modified by:   Qiming Zheng 
 * @Last Modified time: 2022-01-18 20:31:15 
 * @CopyRight: Qiming Zheng 
 */

#pragma once

#include "common.h"
#include "pass/analysis/polyhedral_model.h"

namespace polly {

class Dependency {
 public:
  Dependency(Statement src, Statement dst) : source(src), destination(dst) {}
  Statement source, destination;
};

/// TODOs: implement the dependency graph for fixed amount of synchronizaztion
/// parallelization.
class DependencyGraph {
 public:
  DependencyGraph(PolyhedralModel model);

 private:
  std::vector<Statement> nodes_;
  std::vector<Dependency> edges_;

  std::unordered_map<ArrayKey, std::vector<ArrayAccess>> reads;
  std::unordered_map<ArrayKey, std::vector<ArrayAccess>> writes;
  std::unordered_map<StatementKey, Statement> statements_;
  std::unordered_set<StatementKey> statementKeys_;
  std::unordered_set<ArrayKey> arrayKeys_;
};

}  // namespace polly