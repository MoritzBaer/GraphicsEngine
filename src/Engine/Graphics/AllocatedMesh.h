#pragma once

#include "Mesh.h"

#include "Buffer.h"
#include "Command.h"
#include "UniformAggregate.h"
#include "Util/DeletionQueue.h"
#include <algorithm>
#include <vector>

namespace Engine::Graphics {
class DrawCallRecorder;
class VertexBuffer {
public:
  virtual VkBuffer GetBuffer() const = 0;
  virtual VmaAllocation GetAllocation() const = 0;
};

class GPUObjectManager;
class GPUMemoryManager;

class AllocatedMesh {
protected:
  VertexBuffer *vertexBuffer;
  Buffer<uint32_t> indexBuffer;
  VkDeviceAddress vertexBufferAddress;
  friend class GPUMemoryManager;
  friend class GPUObjectManager;
  friend class DrawCallRecorder;

public:
  AllocatedMesh(VertexBuffer *vertexBuffer, Buffer<uint32_t> const &indexBuffer, VkDeviceAddress vertexBufferAddress)
      : vertexBuffer(vertexBuffer), indexBuffer(indexBuffer), vertexBufferAddress(vertexBufferAddress) {}
  virtual ~AllocatedMesh() {};

  inline void AppendData(PushConstantsAggregate &aggregate) const { aggregate.PushData(&vertexBufferAddress); }
};

template <typename T_GPU> class VertexBufferT : public VertexBuffer {
  Buffer<T_GPU> buffer;
  friend class GPUObjectManager;

public:
  VertexBufferT() : buffer() {}
  VertexBufferT(Buffer<T_GPU> const &buffer) : buffer(buffer) {}
  inline VkBuffer GetBuffer() const override { return buffer.GetBuffer(); }
  inline VmaAllocation GetAllocation() const override { return buffer.GetAllocation(); }
};

} // namespace Engine::Graphics
