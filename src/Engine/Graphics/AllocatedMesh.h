#pragma once

#include "Mesh.h"

#include "Buffer.h"
#include "CommandQueue.h"
#include "Renderer.h"
#include "UniformAggregate.h"
#include "Util/DeletionQueue.h"
#include <algorithm>
#include <vector>

namespace Engine::Graphics {
class AllocatedMesh : public Destroyable {
protected:
  Buffer<uint32_t> indexBuffer;
  VkDeviceAddress vertexBufferAddress;
  bool allocated = false;

public:
  virtual void Upload() = 0;
  virtual void Destroy() const = 0;
  virtual ~AllocatedMesh(){};

  inline void BindAndDraw(VkCommandBuffer const &commandBuffer) const {
    indexBuffer.BindAsIndexBuffer(commandBuffer);
    vkCmdDrawIndexed(commandBuffer, indexBuffer.Size(), 1, 0, 0, 0);
  }
  inline void AppendData(UniformAggregate &aggregate) const { aggregate.PushData(&vertexBufferAddress); }
};

// T_GPU must have a constructor taking a T_CPU const &
template <typename T_CPU, typename T_GPU> class AllocatedMeshT : public AllocatedMesh {

  MeshT<T_CPU, T_GPU> mesh;
  Buffer<T_GPU> vertexBuffer;

public:
  AllocatedMeshT(MeshT<T_CPU, T_GPU> const &mesh) : mesh(mesh) { Upload(); }
  ~AllocatedMeshT() {
    if (allocated) {
      Destroy();
    }
  }
  virtual void Upload();
  virtual void Destroy() const;
  inline AllocatedMeshT<T_CPU, T_GPU> &operator=(MeshT<T_CPU, T_GPU> const &mesh);
};

// Implementations

template <typename T_GPU> class UnstageMeshCommand : public Command {
  BufferCopyCommand vertices;
  BufferCopyCommand indices;

public:
  UnstageMeshCommand(Buffer<uint8_t> stagingBuffer, Buffer<T_GPU> vertexBuffer, Buffer<uint32_t> indexBuffer)
      : vertices(stagingBuffer.CopyTo(vertexBuffer, vertexBuffer.PhysicalSize())),
        indices(stagingBuffer.CopyTo(indexBuffer, indexBuffer.PhysicalSize(), vertexBuffer.PhysicalSize())) {}
  inline void QueueExecution(VkCommandBuffer const &queue) const {
    vertices.QueueExecution(queue);
    indices.QueueExecution(queue);
  }
};

template <typename T_CPU, typename T_GPU> inline void AllocatedMeshT<T_CPU, T_GPU>::Upload() {
  indexBuffer.Create(mesh.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VMA_MEMORY_USAGE_GPU_ONLY);
  vertexBuffer.Create(mesh.vertices.size(),
                      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                      VMA_MEMORY_USAGE_GPU_ONLY);
  vertexBufferAddress = vertexBuffer.GetDeviceAddresss();

  auto uploadReadyVertices(mesh.ReformattedVertices());

  Buffer<uint8_t> stagingBuffer(vertexBuffer.PhysicalSize() + indexBuffer.PhysicalSize(),
                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

  void *data = stagingBuffer.GetMappedData();
  memcpy(data, mesh.ReformattedVertices().data(), vertexBuffer.PhysicalSize());
  memcpy((char *)data + vertexBuffer.PhysicalSize(), mesh.indices.data(), indexBuffer.PhysicalSize());

  auto unstage = UnstageMeshCommand(stagingBuffer, vertexBuffer, indexBuffer);
  Renderer::ImmediateSubmit(&unstage);
  stagingBuffer.Destroy();
  allocated = true;
}

template <typename T_CPU, typename T_GPU> inline void AllocatedMeshT<T_CPU, T_GPU>::Destroy() const {
  indexBuffer.Destroy();
  vertexBuffer.Destroy();
}

template <typename T_CPU, typename T_GPU>
inline AllocatedMeshT<T_CPU, T_GPU> &AllocatedMeshT<T_CPU, T_GPU>::operator=(MeshT<T_CPU, T_GPU> const &mesh) {
  this->mesh = mesh;
  if (allocated) {
    Destroy();
  }
  Upload();
  return *this;
}

} // namespace Engine::Graphics
