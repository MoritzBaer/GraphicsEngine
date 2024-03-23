#include "AllocatedMesh.h"

#include "CommandQueue.h"
#include <algorithm>

namespace Engine::Graphics {

class UnstageMeshCommand : public Command {
  BufferCopyCommand vertices;
  BufferCopyCommand indices;

public:
  UnstageMeshCommand(Buffer<uint8_t> stagingBuffer, Buffer<AllocatedMesh::VertexFormat> vertexBuffer,
                     Buffer<uint32_t> indexBuffer)
      : vertices(stagingBuffer.CopyTo(vertexBuffer, vertexBuffer.PhysicalSize())),
        indices(stagingBuffer.CopyTo(indexBuffer, indexBuffer.PhysicalSize(), vertexBuffer.PhysicalSize())) {}
  inline void QueueExecution(VkCommandBuffer const &queue) const {
    vertices.QueueExecution(queue);
    indices.QueueExecution(queue);
  }
};

void AllocatedMesh::Upload() {
  indexBuffer.Create(mesh.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VMA_MEMORY_USAGE_GPU_ONLY);
  vertexBuffer.Create(mesh.vertices.size(),
                      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                      VMA_MEMORY_USAGE_GPU_ONLY);
  vertexBufferAddress = vertexBuffer.GetDeviceAddresss();

  std::vector<VertexFormat> uploadReadyVertices(mesh.vertices.size());
  std::transform(mesh.vertices.begin(), mesh.vertices.end(), uploadReadyVertices.begin(), [](Mesh::Vertex const &v) {
    return VertexFormat{
        .position = v.position, .uv_x = v.uv[X], .normal = v.normal, .uv_y = v.uv[Y], .colour = v.colour};
  });

  Buffer<uint8_t> stagingBuffer(vertexBuffer.PhysicalSize() + indexBuffer.PhysicalSize(),
                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

  void *data = stagingBuffer.GetMappedData();
  memcpy(data, uploadReadyVertices.data(), vertexBuffer.PhysicalSize());
  memcpy((char *)data + vertexBuffer.PhysicalSize(), mesh.indices.data(), indexBuffer.PhysicalSize());

  auto unstage = UnstageMeshCommand(stagingBuffer, vertexBuffer, indexBuffer);
  Renderer::ImmediateSubmit(&unstage);
  stagingBuffer.Destroy();
  allocated = true;
}

void AllocatedMesh::Destroy() const {
  indexBuffer.Destroy();
  vertexBuffer.Destroy();
}

AllocatedMesh &AllocatedMesh::operator=(Mesh const &mesh) {
  this->mesh = mesh;
  if (allocated) {
    Destroy();
  }
  Upload();
  return *this;
}

} // namespace Engine::Graphics
