//===- ComplementOutputOperators.h -----------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef ONNC_TRANSFORM_COMPLEMENT_OUTPUT_OPERATORS_H
#define ONNC_TRANSFORM_COMPLEMENT_OUTPUT_OPERATORS_H
#include <onnc/Transforms/GraphPairPass.h>

namespace onnc {

/** \class ComplementOutputOperators
 *  \brief ComplementOutputOperators add OutputOperator objects in ComputeGraph.
 */
class ComplementOutputOperators : public GraphPairPass
{
public:
  static char ID;

public:
  ComplementOutputOperators() : GraphPairPass(ID) { }

  ~ComplementOutputOperators() { }

  Pass::ReturnType runOnGraphs(::onnx::Graph& pTG, ComputeGraph& pCG) override;

  StringRef getPassName() const override { return "ComplementOutputOperators"; }
};

ModulePass *CreateComplementOutputOperators();

} // namespace of onnc

#endif