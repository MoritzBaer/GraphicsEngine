#pragma once

#include "Mesh.h"

#include "Buffer.h"
#include "CommandQueue.h"
#include "Renderer.h"
#include "UniformAggregate.h"
#include "Util/DeletionQueue.h"
#include <vector>

namespace Engine::Graphics {
class AllocatedMesh : public Destroyable {
private:
  struct VertexFormat {
    Maths::Vector3 position;
    float uv_x;
    Maths::Vector3 normal;
    float uv_y;
    Maths::Vector4 colour;
  };
  friend class UnstageMeshCommand;

  Mesh mesh;
  Buffer<VertexFormat> vertexBuffer;
  Buffer<uint32_t> indexBuffer;
  VkDeviceAddress vertexBufferAddress;

  bool allocated = false;

public:
  void Upload();
  void Destroy() const;

  AllocatedMesh &operator=(Mesh const &mesh);

  inline void BindAndDraw(VkCommandBuffer const &commandBuffer) const {
    indexBuffer.BindAsIndexBuffer(commandBuffer);
    vkCmdDrawIndexed(commandBuffer, indexBuffer.Size(), 1, 0, 0, 0);
  }
  inline void AppendData(UniformAggregate &aggregate) const { aggregate.PushData(&vertexBufferAddress); }
};

} // namespace Engine::Graphics
