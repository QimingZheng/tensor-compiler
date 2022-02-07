/*
 * @Description: Polly: A DSL compiler for Tensor Program
 * @Author: Qiming Zheng
 * @Date: 2022-01-24 16:02:59
 * @Last Modified by:   Qiming Zheng
 * @Last Modified time: 2022-01-24 16:02:59
 * @CopyRight: Qiming Zheng
 */
#pragma once

#include "common.h"
#include "pass/pass.h"
#include "analysis_pass.h"
#include "polyhedral_extraction.h"
#include "polyhedral_model.h"

namespace polly {

/*!
 * \brief TransformAnalysisPass is the base for each type of transformation
 * analysis.
 */
class TransformAnalysisPass : public Pass {
 public:
  struct Arg : public PassArg {
    static PassArgHandle create() { return std::shared_ptr<Arg>(new Arg); }
  };

  struct Ret : public PassRet {
    bool legal;
    Ret() {}
    Ret(bool x) : legal(x) {}
    static PassRetHandle create(bool x) {
      return std::shared_ptr<Ret>(new Ret(x));
    }
  };
};

/*!
 * \brief Analyze the legality of the Unroll Transformation.
 */
class UnrollTransformAnalysisPass : public TransformAnalysisPass {
 public:
  static PassRetHandle runPass(PassArgHandle arg) { return Ret::create(true); }
};

/*!
 * \brief Analyze the legality of the Split Transformation.
 */
class SplitTransformAnalysisPass : public TransformAnalysisPass {
 public:
  static PassRetHandle runPass(PassArgHandle arg) { return Ret::create(true); }
};

/*!
 * \brief Analyze the legality of the Fission Transformation.
 */
class FissionTransformAnalysisPass : public TransformAnalysisPass {
 public:
  static PassRetHandle runPass(PassArgHandle arg);

  struct Arg : public PassArg {
    IRHandle originalProgram;
    IRHandle transformedProgram;
    Arg() {}
    Arg(IRHandle originalProgram, IRHandle transformedProgram)
        : originalProgram(originalProgram),
          transformedProgram(transformedProgram) {}
    static PassArgHandle create(IRHandle originalProgram,
                                IRHandle transformedProgram) {
      return std::shared_ptr<Arg>(new Arg(originalProgram, transformedProgram));
    }
  };
};

/*!
 * \brief Analyze the legality of the Fussion Transformation.
 */
class FussionTransformAnalysisPass : public TransformAnalysisPass {
 public:
  static PassRetHandle runPass(PassArgHandle arg);

  struct Arg : public PassArg {
    IRHandle originalProgram;
    IRHandle transformedProgram;
    Arg() {}
    Arg(IRHandle originalProgram, IRHandle transformedProgram)
        : originalProgram(originalProgram),
          transformedProgram(transformedProgram) {}
    static PassArgHandle create(IRHandle originalProgram,
                                IRHandle transformedProgram) {
      return std::shared_ptr<Arg>(new Arg(originalProgram, transformedProgram));
    }
  };
};

/*!
 * \brief Analyze the legality of the Reorder Transformation.
 */
class ReorderTransformAnalysisPass : public TransformAnalysisPass {
 public:
  static PassRetHandle runPass(PassArgHandle arg);

  struct Arg : public PassArg {
    IRHandle originalProgram;
    IRHandle transformedProgram;
    Arg() {}
    Arg(IRHandle originalProgram, IRHandle transformedProgram)
        : originalProgram(originalProgram),
          transformedProgram(transformedProgram) {}
    static PassArgHandle create(IRHandle originalProgram,
                                IRHandle transformedProgram) {
      return std::shared_ptr<Arg>(new Arg(originalProgram, transformedProgram));
    }
  };
};

/*!
 * \brief Analyze the legality of the Vectorization Transformation.
 */
class VectorizationTransformAnalysisPass : public TransformAnalysisPass {
 public:
  static PassRetHandle runPass(PassArgHandle arg);
};

}  // namespace polly