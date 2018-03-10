#pragma once

#include <cstdint>
#include <memory>
#include <onnx/common/ir.h>
#include <onnx/onnx_pb.h>
#include <vector>
#include "TGOperator.h"

#define CMD_BUF_NAME "cmdbuf.bin"

namespace {

namespace targetInfo {

// Definition fom BM168xBackendContext.hpp
// TAG will be masked by runtime while processing cmdbuf.
const int GLOBAL_NEURON_TAG = 0x1;
const int GLOBAL_WEIGHT_TAG = 0x2;
const int GLOBAL_ARM_TAG = 0x3;

void ddrScanAndAlloc(MemTable &memTable, onnx::Graph &graph) {
  // allocate spaces for weight
  unsigned int weight_offset = 0;
  // BMKernel only supports DATA_FMT_F32 & DATA_FMT_I1
  int F32_SIZE = 4;
  std::string tab = "\t";

  std::cout << __func__ << " dump global memory layout:" << std::endl;

  for (auto i:graph.initializers()) {

      memTable[i.name()] = weight_offset + GLOBAL_WEIGHT_TAG;
      std::cout << tab << i.name() << " = " << weight_offset;

      assert(i.elem_type() == onnx::TensorProto_DataType_FLOAT);
      if (i.sizes().size() > 0) {
        int tensor_size = F32_SIZE;
        std::cout << " <";
        for(auto dim:i.sizes()) {
          std::cout << dim << ",";
          tensor_size *= dim;
        }
        std::cout << ">" << std::endl;
        weight_offset += tensor_size;
      } else {
        std::cout << std::endl;
      }
  }

  unsigned int neuron_offset = 0;
  std::unordered_set<std::string> initNames(graph.initializer_names().begin(),
                                            graph.initializer_names().end());
  // allocate space for inputs
  for (auto i:graph.inputs()) {
    if(0 == initNames.count(i->uniqueName())) {

      memTable[i->uniqueName()] = neuron_offset + GLOBAL_NEURON_TAG;
      std::cout << tab << i->uniqueName() << " = " << neuron_offset;

      assert(i->elemType() == onnx::TensorProto_DataType_FLOAT);
      if (i->sizes().size() > 0) {
        int tensor_size = F32_SIZE;
        std::cout << " <";
        for (auto &dim : i->sizes()) {
          std::cout << dim.dim << ",";
          tensor_size *= dim.dim;
        }
        std::cout << ">" << std::endl;
        neuron_offset += tensor_size;
      } else {
        std::cout << std::endl;
      }
    }
  }
  // allocate space for outputs
  for (auto i:graph.nodes()) {
    if (i->kind() == onnx::Symbol("Undefined"))
      continue;

    for (auto o:i->outputs()) {

      memTable[o->uniqueName()] = neuron_offset + GLOBAL_NEURON_TAG;
      std::cout << tab << o->uniqueName() << " = " << neuron_offset;

      // FIXME: remove this after output dimension is fixed
      assert(o->elemType() == onnx::TensorProto_DataType_FLOAT);
      if (o->sizes().size() > 0) {
        int tensor_size = F32_SIZE;
        std::cout << " <";
        for(auto dim:o->sizes()) {
          std::cout << dim.dim << ",";
          tensor_size *= dim.dim;
        }
        std::cout << ">" << std::endl;
        neuron_offset += tensor_size;
      } else {
        std::cout << std::endl;
      }
    }
  }
  std::cout << tab << "weight size: " << weight_offset << std::endl;
  std::cout << tab << "neuron size: " << neuron_offset << std::endl;
}
} // end of targetInfo namespace

// TODO make as onnx optimization pass
namespace updateOutputInfoPass {

void updateOutputsInfo(onnx::ArrayRef<onnx::Value *> &&outputs,
                      const std::vector<onnx::Dimension> &dims,
                      onnx::TensorProto_DataType type) {
  for (auto outVal : outputs) {
    outVal->setElemType(type);
    if (0 == outVal->sizes().size())
      outVal->setSizes(dims);
  }
}

void updateOutputInfoByInput(onnx::Node *const node) {
  auto input = node->inputs()[0];
  const std::vector<onnx::Dimension> inputDim = input->sizes();
  const onnx::TensorProto_DataType inputType = input->elemType();
  assert(inputType != onnx::TensorProto_DataType_UNDEFINED);
  // FIXME workaorund unimplemented type
  if (0 == inputDim.size())
    return;
  updateOutputsInfo(node->outputs(), inputDim, inputType);
}

void updateConvOutputDim(onnx::Node *const node) {
  const std::vector<onnx::Dimension> inputDim = node->inputs()[0]->sizes();
  // FIXME workaorund unimplemented type
  if (0 == inputDim.size())
    return;
  const auto iN = inputDim[0].dim;
  const auto iC = inputDim[1].dim;
  const auto iH = inputDim[2].dim;
  const auto iW = inputDim[3].dim;

  const auto weightDim = node->inputs()[1]->sizes();
  const auto wN = weightDim[0].dim;
  const auto wC = weightDim[1].dim;
  auto kH = weightDim[2].dim;
  auto kW = weightDim[3].dim;

  int64_t sH(1), sW(1);
  // pads for x_begin, y_begin, x_end, y_end
  int64_t xb(0), yb(0), xe(0), ye(0);

  if (node->hasAttribute(onnx::Symbol("kernel_shape"))) {
    auto &i = node->is(onnx::Symbol("kernel_shape"));
    kH = i[0];
    kW = i[1];
  }

  if (node->hasAttribute(onnx::Symbol("strides"))) {
    auto &i = node->is(onnx::Symbol("strides"));
    sH = i[0];
    sW = i[1];
  }

  if (node->hasAttribute(onnx::Symbol("pads"))) {
    auto &i = node->is(onnx::Symbol("pads"));
    xb = i[0];
    yb = i[1];
    xe = i[2];
    ye = i[3];
  }

  int64_t oN = iN;
  int64_t oC = wN;
#if 1
  // NOTE: It is for bmkernel only padding on both ends
  int64_t oH = (iH - kH + 2 * xb) / sH + 1;
  int64_t oW = (iW - kW + 2 * yb) / sW + 1;
#else
  int64_t oH = (iH - kH + xb + xe) / sH + 1;
  int64_t oW = (iW - kW + yb + ye) / sW + 1;
#endif

  std::vector<onnx::Dimension> outDims{ onnx::Dimension(oN),
                                        onnx::Dimension(oC),
                                        onnx::Dimension(oH),
                                        onnx::Dimension(oW) };

  const onnx::TensorProto_DataType inputType = node->inputs()[0]->elemType();
  updateOutputsInfo(node->outputs(), outDims, inputType);
}

void updatePoolOutputDim(onnx::Node *const node) {
  const std::vector<onnx::Dimension> inputDim = node->inputs()[0]->sizes();
  // FIXME workaorund unimplemented type
  if (0 == inputDim.size())
    return;
  const auto iN = inputDim[0].dim;
  const auto iC = inputDim[1].dim;
  const auto iH = inputDim[2].dim;
  const auto iW = inputDim[3].dim;
  const auto kH = node->is(onnx::Symbol("kernel_shape"))[0];
  const auto kW = node->is(onnx::Symbol("kernel_shape"))[1];

  int64_t sH(1), sW(1);
  // pads for x_begin, y_begin, x_end, y_end
  int64_t xb(0), yb(0), xe(0), ye(0);

  if (node->hasAttribute(onnx::Symbol("strides"))) {
    auto &i = node->is(onnx::Symbol("strides"));
    sH = i[0];
    sW = i[1];
  }

  if (node->hasAttribute(onnx::Symbol("pads"))) {
    auto &i = node->is(onnx::Symbol("pads"));
    xb = i[0];
    yb = i[1];
    xe = i[2];
    ye = i[3];
  }

  int64_t oN = iN;
  int64_t oC = iC;
#if 1
  // NOTE: It is for bmkernel only padding on both ends
  int64_t oH = static_cast<int64_t>(ceil(static_cast<float>(
      iH - kH + 2 * xb) / sH)) + 1;
  int64_t oW = static_cast<int64_t>(ceil(static_cast<float>(
      iW - kW + 2 * yb) / sW)) + 1;
#else
  int64_t oH = static_cast<int64_t>(ceil(static_cast<float>(
      iH - kH + xb + xe) / sH)) + 1;
  int64_t oW = static_cast<int64_t>(ceil(static_cast<float>(
      iW - kW + yb + ye) / sW)) + 1;
#endif

  std::vector<onnx::Dimension> outDims{ onnx::Dimension(oN),
                                        onnx::Dimension(oC),
                                        onnx::Dimension(oH),
                                        onnx::Dimension(oW) };

  const onnx::TensorProto_DataType inputType = node->inputs()[0]->elemType();
  updateOutputsInfo(node->outputs(), outDims, inputType);
}

void updateGemmOutputDim(onnx::Node *const node) {

  const std::vector<onnx::Dimension> aDim = node->inputs()[0]->sizes();
  const std::vector<onnx::Dimension> bDim = node->inputs()[1]->sizes();
  // FIXME workaorund unimplemented type
  if (0 == aDim.size() || 0 == bDim.size())
    return;
  // A: M x K
  int64_t oM = aDim[0].dim;
  if (node->hasAttribute(onnx::Symbol("transA"))) {
    auto transA = node->i(onnx::Symbol("transA"));
    oM = transA ? aDim[1].dim : aDim[0].dim;
  }
  // B: K x N
  int64_t oN = bDim[1].dim;
  if (node->hasAttribute(onnx::Symbol("transB"))) {
    auto transB = node->i(onnx::Symbol("transB"));
    oN = transB ? bDim[0].dim : bDim[1].dim;
  }

  std::vector<onnx::Dimension> outDims{ onnx::Dimension(oM),
                                        onnx::Dimension(oN) };
  const onnx::TensorProto_DataType inputType = node->inputs()[0]->elemType();
  updateOutputsInfo(node->outputs(), outDims, inputType);
}

void updateOutputInfo(onnx::Graph &graph) {
  for (onnx::graph_node_list_iterator it = graph.begin(), ie = graph.end();
       it != ie; ++it) {
    onnx::Node *node = *it;
    auto symbol = node->kind();

    if (symbol == onnx::Symbol("Conv")) {
      updateConvOutputDim(node);
    } else if (symbol == onnx::Symbol("Relu")) {
      updateOutputInfoByInput(node);
    } else if (symbol == onnx::Symbol("LRN")) {
      updateOutputInfoByInput(node);
    } else if (symbol == onnx::Symbol("MaxPool")) {
      updatePoolOutputDim(node);
    } else if (symbol == onnx::Symbol("Dropout")) {
      updateOutputInfoByInput(node);
    } else if (symbol == onnx::Symbol("Gemm")) {
      updateGemmOutputDim(node);
    } else if (symbol == onnx::Symbol("Softmax")) {
      updateOutputInfoByInput(node);
    } else {
      std::cerr << "unimplemented type: " << symbol.toString() << std::endl;
    }
  }
}
} // end updateOutputInfoPass namespace


// remove unsed node in reference
namespace removeUnusedNodePass {

void removeUnusedNode(onnx::Graph &graph) {
  for (auto it = graph.begin(), ie = graph.end(); it != ie; ++it) {
    auto *node = *it;
    auto symbol = node->kind();
    if (symbol == onnx::Symbol("Dropout")) {
      // Dropout has multiple outputs
      node->outputs()[0]->replaceAllUsesWith(node->input());
      it.destroyCurrent();
    }
  }
}

} // end removeUnusedNodePass namespace


} // end anonymous namespace

class TGBackend {
public:
  TGBackend(const onnx::ModelProto &model);
  ~TGBackend();
  TGBackend &lowering(void);
  void codeEmit(void);

private:
  static void sendCmdBuf(void *userData, const void *cmdBuf, uint32_t len);
  static void emitCmdBuf(void *userData, void *cmdBuf, uint32_t len);
  static void freeCmdBuf(void *userData, void *cmdBuf);
  static void *allocCmdBuf(void *userData, uint32_t size);
  static void hostSync(void);
  static void emitDebugInfo(void *userData, char const *info, int nodeId,
                            long long unsigned int fwAddr, bool isFloat);
  void bmkernelContextPrepare(void);

  void *m_bmkernelHandle;
  std::shared_ptr<onnx::Graph> m_onnxGraph;
  MemTable m_globalMemLayout;
  std::vector<std::unique_ptr<TGOperator>> m_instructions;
};
