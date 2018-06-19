//===- MemoryAllocation.h -------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef ONNC_MEMORY_ALLOCATION_H
#define ONNC_MEMORY_ALLOCATION_H
#include <onnc/Core/ModulePass.h>
#include <onnc/Core/PassSupport.h>
#include <onnc/Target/TargetMemInfo.h>
#include <onnx/common/ir.h>
#include <vector>

namespace onnc {

class LiveInterval;
class DLATargetBackend;

struct MemAllocEntry
{
public:
  MemAllocEntry(size_t pStartAddr, size_t pSize, const LiveInterval &pIntrvl)
    : startAddr(pStartAddr), size(pSize), liveIntrvl(pIntrvl) {
  }

  size_t startAddr, size;
  const LiveInterval liveIntrvl;
};

typedef std::vector<MemAllocEntry*> MemAllocList;
typedef std::unordered_map<const ::onnx::Value *, MemSize> ValMemSizeMap;

/** \class MemoryAllocation
 *  Perform memory allocation and generate allocation map.
 */
class MemoryAllocation : public ModulePass
{
public:
  typedef std::unordered_map<::onnx::Graph *, MemAllocList> GraphMemAllocList;

  static char ID;

  virtual ~MemoryAllocation();

public:
  MemoryAllocation(DLATargetBackend* pDLATB = nullptr);

  ReturnType runOnModule(Module& pModule) override;

  void getAnalysisUsage(AnalysisUsage& pUsage) const override;

  void printGraphAlloc(OStream &pOS, const ::onnx::Graph *pGraph) const;

  void print(OStream& pOS) const;

private:
  /// Return total size of this allocation.
  uint64_t allocByLiveness(::onnx::Graph &pGraph,
                           ValMemSizeMap &pValMemSizeMap,
                           GraphLivenessAnalysis &liveAnaly);

  /// delete MemAllocEntries of graph.
  void clearGraphAlloc(::onnx::Graph *pGraph);

  void clear();

private:
  GraphMemAllocList m_GraphMemAllocList;
  DLATargetBackend* m_DLATB = nullptr;
};

MemoryAllocation* CreateMemoryAllocationPass(DLATargetBackend* pDLATB);

} // namespace of onnc

#endif
