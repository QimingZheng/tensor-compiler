#pragma once

#include "common.h"
#include "pass/pass.h"
#include "analysis_pass.h"
#include "polyhedral_extraction.h"
#include "polyhedral_model.h"

namespace polly {

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

class UnrollTransformAnalysisPass : public TransformAnalysisPass {
 public:
  static PassRetHandle runPass(PassArgHandle arg) { return Ret::create(true); }
};

class SplitTransformAnalysisPass : public TransformAnalysisPass {
 public:
  static PassRetHandle runPass(PassArgHandle arg) { return Ret::create(true); }
};

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

class VectorizationTransformAnalysisPass : public TransformAnalysisPass {
 public:
  static PassRetHandle runPass(PassArgHandle arg);
};

}  // namespace polly