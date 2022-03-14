/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-18 20:33:00
 * @Last Modified by: Qiming Zheng
 * @Last Modified time: 2022-01-26 10:20:54
 * @CopyRight: Qiming Zheng
 */
#pragma once

#include "common.h"
#include "ir/ir_module.h"
#include "codegen/codegen.h"
#include "arch_spec.h"

namespace polly {

/// Evaluate the performance of an program.
class CostModel {
 public:
  CostModel() {}

  float Evaluate(IRModule space, ArchSpec spec, std::string program_name) {
    std::string res;

    switch (spec.type_) {
      case ArchSpec::ArchType::CPU: {
        std::string costModelFileName = ".polly_cost_model.cc";
        std::ofstream f;
        f.open(costModelFileName);
        {
          CodeGenC codegen;
          f << codegen.genCode(space.GetRoot(), space.GetTensors(),
                               program_name);
        }
        {
          CodeGenC codegen;
          f << codegen.genTensors(space.GetTensors());
        }
        f << "int main() {\n";
        {
          CodeGenC codegen;
          f << "  " << program_name << "("
            << codegen.genTensorParam(space.GetTensors()) << ");\n";
        }

        f << "  struct timeval start, end;\n";
        f << "  gettimeofday(&start, NULL);\n";
        f << "  for (int step = 0; step < 3; step ++) {\n";

        {
          CodeGenC codegen;
          f << "    " << program_name << "("
            << codegen.genTensorParam(space.GetTensors()) << ");\n";
        }
        f << "  }\n";
        // timing unit: ms
        f << "  gettimeofday(&end, NULL);\n";
        f << "  printf(\"%.6f\\n\", ((end.tv_sec - start.tv_sec) * 1000L + "
             "(end.tv_usec - start.tv_usec) * 1.0 / 1000L) / 3.0);\n";

        f << "}\n";
        f.close();

        // Compile

        res = executeCommands(
            "g++ --std=c++11 -O3 -mfma -fopenmp -o .main .polly_cost_model.cc");

        // Execute & get running time
        res = executeCommands("timeout 400 ./.main");
        break;
      }
      default:
        throw std::runtime_error("Not supported architecture.");
    }
    // std::cout << "time: " << res;
    if (res == "") {
      CodeGenC codegen;
      std::cout << codegen.genCode(space.GetRoot(), space.GetTensors(),
                                   program_name);
      return 1000000000.0;
    }
    if (!isLegalNumber(res)) return 000000000.0;
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

  bool isLegalNumber(std::string str) {
    for (int i = 0; i < str.length(); i++) {
      if (str[i] > '9' && str[i] < '0' && str[i] != '.') return false;
    }
    return true;
  }
};

}  // namespace  polly
