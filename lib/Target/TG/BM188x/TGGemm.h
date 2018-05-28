#ifndef ONNX_BM1880_TGGEMM_H
#define ONNX_BM1880_TGGEMM_H

#include "ComputeOperand.h"
#include "common_calibration.pb.h"
#include <onnx/common/ir.h>

namespace onnc {
namespace BM188X {

// m_MemOperands: input, weight, bias, output
class TGGemm : public ComputeOperand2
{
public:
  TGGemm(const ::onnx::Node &pNode,
         const LayerCalibrationParameter &pLayerCtable);
  void emit() const override;
  void print(OStream &pOS) const override;

private:
  int m_inRowNum;
  int m_inColNum;
  int m_outColNum;
  int m_haveBias;
  bool m_weightTp;
  LayerCalibrationParameter m_LayerCtable;
};

} // namespace BM188X
} // namespace onnc

#endif
