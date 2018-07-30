//===- SoftsignLower.cpp -------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <onnc/Transforms/TensorSel/Lower.h>
#include <onnc/Transforms/TensorSel/Standards/SoftsignLower.h>
#include <onnc/IR/Compute/Softsign.h>
#include <onnc/IR/IRBuilder.h>

using namespace onnc;

//===----------------------------------------------------------------------===//
// SoftsignLower
//===----------------------------------------------------------------------===//
SoftsignLower::SoftsignLower()
{
}

SoftsignLower::~SoftsignLower()
{
}

int SoftsignLower::isMe(const ::onnx::Node& pNode) const
{
  if (pNode.kind() == ::onnx::Symbol("Softsign"))
    return kStdLower;
  return kNotMe;
}

ComputeOperator*
SoftsignLower::activate(ComputeGraph& pGraph, ::onnx::Node& pNode) const
{
  // check input/output number
  if (pNode.inputs().size() != 1)
    return nullptr;

  if (pNode.outputs().size() != 1)
    return nullptr;

  // check input/output name
  for (::onnx::Value* xv : pNode.inputs()) {
    if (!xv->has_unique_name())
      return nullptr;
  }

  for (::onnx::Value* xv : pNode.outputs()) {
    if (!xv->has_unique_name())
      return nullptr;
  }

  // check default attributes
  

  // create operators
  onnc::Softsign* op = pGraph.addOperator<onnc::Softsign>();

  // set optional attributes
  

  // set input/output
  for (::onnx::Value* xv : pNode.inputs()) {
    onnc::Tensor* tensor = pGraph.getValue<onnc::Tensor>(xv->uniqueName());
    if (nullptr == tensor)
      tensor = IRBuilder::CreateComputeTensor(pGraph, *xv);
    op->addInput(*tensor);
  }

  for (::onnx::Value* xv : pNode.outputs()) {
    onnc::Tensor* tensor = pGraph.getValue<onnc::Tensor>(xv->uniqueName());
    if (nullptr == tensor)
      tensor = IRBuilder::CreateComputeTensor(pGraph, *xv);
    op->addOutput(*tensor);
  }
  return op;
}
