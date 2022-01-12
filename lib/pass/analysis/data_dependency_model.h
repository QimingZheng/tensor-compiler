#pragma once

#include "common.h"
#include "polyhedral_model.h"
#include "polyhedral_extraction.h"
#include "solver/solver.h"

namespace polly {

class DataDependencyModel {
 public:
  DataDependencyModel(PolyhedralModel model);

 private:
  PolyhedralModel model;
};

}  // namespace polly