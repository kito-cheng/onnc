//===- LSTM.cpp ------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <onnc/IR/Compute/LSTM.h>

using namespace onnc;

//===----------------------------------------------------------------------===//
// LSTM
//===----------------------------------------------------------------------===//
LSTM::LSTM()
  : ComputeOperator("LSTM"),
    m_ActivationAlpha(),
    m_ActivationBeta(),
    m_Activations(),
    m_Clip(),
    m_Direction(),
    m_HiddenSize(),
    m_InputForget(),
    m_OutputSequence() {
}

LSTM::LSTM(const FloatsAttr& pActivationAlpha,
           const FloatsAttr& pActivationBeta,
           const StringsAttr& pActivations,
           const FloatAttr& pClip,
           const StringAttr& pDirection,
           const IntAttr& pHiddenSize,
           const IntAttr& pInputForget,
           const IntAttr& pOutputSequence)
  : ComputeOperator("LSTM"),
    m_ActivationAlpha(pActivationAlpha),
    m_ActivationBeta(pActivationBeta),
    m_Activations(pActivations),
    m_Clip(pClip),
    m_Direction(pDirection),
    m_HiddenSize(pHiddenSize),
    m_InputForget(pInputForget),
    m_OutputSequence(pOutputSequence) {
}

void LSTM::print(std::ostream& pOS) const
{
}
