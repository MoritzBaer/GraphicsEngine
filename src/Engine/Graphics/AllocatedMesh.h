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
class AllocatedMesh {
protected:
  Buffer<uint32_t> indexBuffer;
  VkDeviceAddress vertexBufferAddress;

public:
  AllocatedMesh(Buffer<uint32_t> const &indexBuffer, VkDeviceAddress vertexBufferAddress)
      : indexBuffer(indexBuffer), vertexBufferAddress(vertexBufferAddress) {}
  virtual ~AllocatedMesh() {};

  inline void BindAndDraw(VkCommandBuffer const &commandBuffer) const {
    indexBuffer.BindAsIndexBuffer(commandBuffer);
    vkCmdDrawIndexed(commandBuffer, indexBuffer.Size(), 1, 0, 0, 0);
  }
  inline void AppendData(PushConstantsAggregate &aggregate) const { aggregate.PushData(&vertexBufferAddress); }
};

// T_GPU must have a constructor taking a T_CPU const &
template <typename T_GPU> class AllocatedMeshT : public AllocatedMesh {
  Buffer<T_GPU> vertexBuffer;

public:
  AllocatedMeshT(Buffer<T_GPU> const &vertexBuffer, Buffer<uint32_t> const &indexBuffer,
                 VkDeviceAddress vertexBufferAddress)
      : AllocatedMesh(indexBuffer, vertexBufferAddress), vertexBuffer(vertexBuffer) {}
  AllocatedMeshT(AllocatedMeshT<T_GPU> const &other)
      : AllocatedMeshT(other.vertexBuffer, other.indexBuffer, other.vertexBufferAddress) {}
  ~AllocatedMeshT() {}
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
