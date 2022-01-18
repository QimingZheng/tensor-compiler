/*
 * @Description: Polly: A DSL compiler for Tensor Program 
 * @Author: Qiming Zheng 
 * @Date: 2022-01-18 20:33:00 
 * @Last Modified by:   Qiming Zheng 
 * @Last Modified time: 2022-01-18 20:33:00 
 * @CopyRight: Qiming Zheng 
 */
#pragma once

#include "common.h"
#include "ir/ir_module.h"

namespace polly {

/// Evaluate the performance of an program.
class CostModel {
 public:
  CostModel() {}

  float Evaluate(IRModule space) {
    std::string costModelFileName = ".polly_cost_model.cc";
    std::ofstream f;
    f.open(costModelFileName);
    CodeGenC codegen(f);
    codegen.genCode(space.GetRoot(), space.GetTensors());
    f.close();

    // Compile
    executeCommands("g++ --std=c++11 -O3  -o .main .polly_cost_model.cc");
    // Execute & get running time
    auto res = executeCommands("./.main");
    std::cout << res;

    float runtime = atof(res.c_str());
    return runtime;
  }

 private:
  std::string executeCommands(std::string cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
      while (fgets(buffer, sizeof buffer, pipe) != NULL) {
        result += buffer;
      }
    } catch (...) {
      pclose(pipe);
      throw std::runtime_error("cannot read command execution result");
    }
    pclose(pipe);
    return result;
  }
};

}  // namespace  polly
