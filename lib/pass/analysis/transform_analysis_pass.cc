#include "transform_analysis_pass.h"

namespace polly {

bool TransformAnalysisPassHelper(IRHandle originalProgram,
                                 IRHandle transformedProgram) {
  solver::context ctx;
  PolyhedralModel oriModel = PolyhedralExtraction(originalProgram).model;
  PolyhedralModel transformedModel =
      PolyhedralExtraction(transformedProgram).model;

  auto ret = PolyhedralAnalysisPass::runPass(
      std::shared_ptr<PolyhedralAnalysisPass::Arg>(
          new PolyhedralAnalysisPass::Arg(
              ctx, originalProgram, transformedProgram,
              DataDependencyModel::CreateTransformMap(ctx, oriModel,
                                                      transformedModel))));

  return !(PassRet::as<PolyhedralAnalysisPass::Ret>(ret)->hasConflicts);
}

PassRetHandle FissionTransformAnalysisPass::runPass(PassArgHandle arg) {
  auto originalProgram = PassArg::as<Arg>(arg)->originalProgram;
  auto transformedProgram = PassArg::as<Arg>(arg)->transformedProgram;

  return Ret::create(
      TransformAnalysisPassHelper(originalProgram, transformedProgram));
}

PassRetHandle FussionTransformAnalysisPass::runPass(PassArgHandle arg) {
  auto originalProgram = PassArg::as<Arg>(arg)->originalProgram;
  auto transformedProgram = PassArg::as<Arg>(arg)->transformedProgram;

  return Ret::create(
      TransformAnalysisPassHelper(originalProgram, transformedProgram));
}

PassRetHandle ReorderTransformAnalysisPass::runPass(PassArgHandle arg) {
  auto originalProgram = PassArg::as<Arg>(arg)->originalProgram;
  auto transformedProgram = PassArg::as<Arg>(arg)->transformedProgram;

  return Ret::create(
      TransformAnalysisPassHelper(originalProgram, transformedProgram));
}

}  // namespace polly
