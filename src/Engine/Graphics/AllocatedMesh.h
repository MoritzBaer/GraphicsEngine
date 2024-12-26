#pragma once

#include "Mesh.h"

#include "Buffer.h"
#include "CommandQueue.h"
#include "GPUMemoryManager.h"
#include "UniformAggregate.h"
#include "Util/DeletionQueue.h"
#include <algorithm>
#include <vector>

namespace Engine::Graphics {

class VertexBuffer {
public:
  virtual VkBuffer GetBuffer() const = 0;
  virtual VmaAllocation GetAllocation() const = 0;
};

class AllocatedMesh {
protected:
  VertexBuffer *vertexBuffer;
  Buffer<uint32_t> indexBuffer;
  VkDeviceAddress vertexBufferAddress;
  friend class GPUMemoryManager;
  friend class GPUObjectManager;

public:
  AllocatedMesh(VertexBuffer *vertexBuffer, Buffer<uint32_t> const &indexBuffer, VkDeviceAddress vertexBufferAddress)
      : vertexBuffer(vertexBuffer), indexBuffer(indexBuffer), vertexBufferAddress(vertexBufferAddress) {}
  virtual ~AllocatedMesh() {};

  inline void BindAndDraw(VkCommandBuffer const &commandBuffer) const {
    indexBuffer.BindAsIndexBuffer(commandBuffer);
    vkCmdDrawIndexed(commandBuffer, indexBuffer.Size(), 1, 0, 0, 0);
  }
  inline void AppendData(PushConstantsAggregate &aggregate) const { aggregate.PushData(&vertexBufferAddress); }
};

template <typename T_GPU> class VertexBufferT : public VertexBuffer {
  Buffer<T_GPU> buffer;

public:
  VertexBufferT(Buffer<T_GPU> const &buffer) : buffer(buffer) {}
  inline VkBuffer GetBuffer() const override { return buffer.GetBuffer(); }
  inline VmaAllocation GetAllocation() const override { return buffer.GetAllocation(); }
};

// Implementations

template <typename T_GPU> class UnstageMeshCommand : public Command {
  BufferCopyCommand vertices;
  BufferCopyCommand indices;

public:
  UnstageMeshCommand(Buffer<uint8_t> stagingBuffer, Buffer<T_GPU> vertexBuffer, Buffer<uint32_t> indexBuffer)
      : vertices(GPUMemoryManager::CopyBufferToBuffer(stagingBuffer, vertexBuffer, vertexBuffer.PhysicalSize())),
        indices(GPUMemoryManager::CopyBufferToBuffer(stagingBuffer, indexBuffer, indexBuffer.PhysicalSize(),
                                                     vertexBuffer.PhysicalSize())) {}
  inline void QueueExecution(VkCommandBuffer const &queue) const {
    vertices.QueueExecution(queue);
    indices.QueueExecution(queue);
  }
};

} // namespace Engine::Graphics
