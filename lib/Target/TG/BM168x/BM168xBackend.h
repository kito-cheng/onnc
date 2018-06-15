//===- BM168xBackend.h ----------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef BM168X_BACKEND_H
#define BM168X_BACKEND_H
#include "TGBackend.h"
#include <memory>
#include <onnx/common/ir.h>
#include <string>

namespace onnc {

class TargetLowering;
class TGCodeEmitter;

class BM1680Backend : public TGBackend
{
public:
  BM1680Backend(const TargetOptions &pOptions);
  ~BM1680Backend() override;
  bool isNativeTensorType(::onnx::TensorProto_DataType pType) override;
  std::string getBackendName() override
  {
    return std::string("BM1680Backend");
  };
};

class BM1682Backend : public TGBackend
{
public:
  BM1682Backend(const TargetOptions &pOptions);
  ~BM1682Backend() override;
  bool isNativeTensorType(::onnx::TensorProto_DataType pType) override;
  std::string getBackendName() override
  {
    return std::string("BM1682Backend");
  };
};

} // namespace onnc

#endif // BM168X_BACKEND_H
