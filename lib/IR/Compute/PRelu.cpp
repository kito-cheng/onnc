//===- PRelu.cpp ------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <onnc/IR/Compute/PRelu.h>

using namespace onnc;

char PRelu::ID = 0;

//===----------------------------------------------------------------------===//
// PRelu
//===----------------------------------------------------------------------===//
PRelu::PRelu()
  : ComputeOperator("PRelu", ID) {
}



void PRelu::print(std::ostream& pOS) const
{
}

bool PRelu::classof(const ComputeOperator* pOp)
{
  if (nullptr == pOp)
    return false;
  return (pOp->getID() == &ID);
}
