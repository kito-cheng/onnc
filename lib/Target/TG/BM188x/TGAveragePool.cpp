#define DEBUG_TYPE "tg_averagepool"
#include "TGAveragePool.h"
#include "BM188xCodeEmitter.h"
#include <bmkernel_api.h>
#include <onnc/Support/Debug.h>

namespace onnc {
namespace BM188X {

TGAveragePool::TGAveragePool(const ::onnx::Node &pNode)
    : BM188xComputeOperator(pNode, std::string("AveragePool")), m_PadH(0),
      m_PadW(0), m_StrideH(1), m_StrideW(1), m_EnableRelu(0), m_RShiftWidth(0),
      m_ThresholdXQuantized(0)
{
  const std::vector< ::onnx::Dimension> inDim = pNode.inputs()[0]->sizes();
  m_N = inDim[0].dim;
  m_C = inDim[1].dim;
  m_H = inDim[2].dim;
  m_W = inDim[3].dim;
  m_KH = pNode.is(::onnx::Symbol("kernel_shape"))[0];
  m_KW = pNode.is(::onnx::Symbol("kernel_shape"))[1];

  // [leftPad, downPad, rightPad, upPad]
  if (pNode.hasAttribute(::onnx::Symbol("pads"))) {
    auto &i = pNode.is(::onnx::Symbol("pads"));
    // NOTE: It is for bmkernel padding on both ends
    m_PadH = i[0];
    m_PadW = i[1];
  }
  if (pNode.hasAttribute(::onnx::Symbol("strides"))) {
    auto &i = pNode.is(::onnx::Symbol("strides"));
    m_StrideH = i[0];
    m_StrideW = i[1];
  }
  if (pNode.hasAttribute(::onnx::Symbol("enable_relu"))) {
    m_EnableRelu = pNode.i(::onnx::Symbol("enable_relu"));
  }
}

TGAveragePool *TGAveragePool::addMemOperands(MemOperand *pInput,
                                             MemOperand *pOutput)
{
  m_MemOperands.push_back(pInput);
  m_MemOperands.push_back(pOutput);
  return this;
}

void TGAveragePool::print(OStream &pOS) const
{
  pOS << *m_MemOperands[1] << " = AveragePool <N:" << m_N << ", C:" << m_C
      << ", H:" << m_H << ", W:" << m_W << ",  kH:" << m_KH << ", kW:" << m_KW
      << ", padH:" << m_PadH << ", padW:" << m_PadW << ", srideH:" << m_StrideH
      << ", strideW:" << m_StrideW << ", rShiftWidth:" << m_RShiftWidth
      << ", thresholdX:" << m_ThresholdXQuantized << "> (" << *m_MemOperands[0]
      << ")\n";
}

void TGAveragePool::emit() const
{
  DEBUG(print(dbgs()));

  bmnet::bmnet_pooling_fixed_forward_bmkernel(
      *bm1880_kernel::getInstance().m_CTX,
      m_MemOperands[0]->m_Addr, // ifmap_gaddr
      m_MemOperands[1]->m_Addr, // ofmap_gaddr
      GADDR_INVALID,            // index_gaddr
      GADDR_INVALID,            // o_findex_gaddr
      m_N, m_C, m_H, m_W, m_KH, m_KW, m_PadH, m_PadW, m_StrideH, m_StrideW,
      1,                     // is_avg_pooling
      0.0f,                  // avg_const
      m_EnableRelu,          // do_relu
      m_RShiftWidth,         // right_shift_width
      &m_ThresholdXQuantized // threshold_x_quantized
  );
}

void TGAveragePool::toASM(tg::bm1880::Inst *pI) const
{
  pI->set_name(getLayerName());
  pI->set_type("bmnet_pooling_fixed_forward_bmkernel");
  {
    auto *pool = pI->mutable_pooling();
    pool->set_ifmap_gaddr(m_MemOperands[0]->m_Addr);
    pool->set_ofmap_gaddr(m_MemOperands[1]->m_Addr);
    pool->set_n(m_N);
    pool->set_c(m_C);
    pool->set_h(m_H);
    pool->set_w(m_W);
    pool->set_kh(m_KH);
    pool->set_kw(m_KW);
    pool->set_pad_h(m_PadH);
    pool->set_pad_w(m_PadW);
    pool->set_stride_h(m_StrideH);
    pool->set_stride_w(m_StrideW);
    pool->set_is_avg_pooling(1);
    pool->set_avg_const(0.0f);
    pool->set_do_relu(m_EnableRelu);
    pool->set_right_shift_width(m_RShiftWidth);
    pool->set_threshold_x_quantized(m_ThresholdXQuantized);
  }
}

void TGAveragePool::update(
    const tg::bm1880::LayerCalibrationParameter *pLayerCtable)
{
  m_RShiftWidth = pLayerCtable->right_shift_width();
  m_ThresholdXQuantized = *pLayerCtable->threshold_x_quantized().data();
}

} // namespace BM188X
} // namespace onnc