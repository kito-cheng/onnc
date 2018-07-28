//===- Div.cpp ------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <onnc/IR/Compute/Div.h>

using namespace onnc;

char Div::ID = 0;

//===----------------------------------------------------------------------===//
// Div
//===----------------------------------------------------------------------===//
Div::Div()
  : ComputeOperator("Div", ID),
    m_Axis(),
    m_Broadcast() {
}

Div::Div(const IntAttr& pAxis,
         const IntAttr& pBroadcast)
  : ComputeOperator("Div", ID),
    m_Axis(pAxis),
    m_Broadcast(pBroadcast) {
}

void Div::print(std::ostream& pOS) const
{
}

bool Div::classof(const ComputeOperator* pOp)
{
  if (nullptr == pOp)
    return false;
  return (pOp->getID() == &ID);
}
