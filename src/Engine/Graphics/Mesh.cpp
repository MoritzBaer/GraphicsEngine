#include "Mesh.h"
#include "Renderer.h"

namespace Engine::Graphics
{
    class UnstageMeshCommand : public Command 
    {
        BufferCopyCommand vertices;
        BufferCopyCommand indices;
    public:
        UnstageMeshCommand(Buffer<uint8_t> stagingBuffer, Buffer<Mesh::VertexFormat> vertexBuffer, Buffer<uint32_t> indexBuffer) 
            : vertices(stagingBuffer.CopyTo(vertexBuffer, vertexBuffer.PhysicalSize())), indices(stagingBuffer.CopyTo(indexBuffer, indexBuffer.PhysicalSize(), vertexBuffer.PhysicalSize())) { }
        inline void QueueExecution(VkCommandBuffer const & queue) const {
            vertices.QueueExecution(queue);
            indices.QueueExecution(queue);
        }
    };

    void Mesh::Upload()
    {
        gpuBuffers.indexBuffer.Create(indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        gpuBuffers.vertexBuffer.Create(vertices.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        gpuBuffers.vertexBufferAddress = gpuBuffers.vertexBuffer.GetDeviceAddresss();

        Buffer<uint8_t> stagingBuffer(gpuBuffers.vertexBuffer.PhysicalSize() + gpuBuffers.indexBuffer.PhysicalSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

        void * data = stagingBuffer.GetMappedData();
        memcpy(data, vertices.data(), gpuBuffers.vertexBuffer.PhysicalSize());
        memcpy((char*)data + gpuBuffers.vertexBuffer.PhysicalSize(), indices.data(), gpuBuffers.indexBuffer.PhysicalSize());

        auto unstage = UnstageMeshCommand(stagingBuffer, gpuBuffers.vertexBuffer, gpuBuffers.indexBuffer);
        Renderer::ImmediateSubmit(&unstage);
        stagingBuffer.Destroy();
    }

    void Mesh::Destroy()
    {
        gpuBuffers.indexBuffer.Destroy();
        gpuBuffers.vertexBuffer.Destroy();
    }

    void Mesh::DrawMeshCommand::QueueExecution(VkCommandBuffer const &queue) const
    {
        Mesh::GPUPushConstants pushConstants {
            .modelMatrix = Maths::Matrix4::Identity(),
            .vertexBufferAddress = buffers.vertexBufferAddress
        };

        vkCmdPushConstants(queue, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mesh::GPUPushConstants), &pushConstants);
        buffers.indexBuffer.BindAsIndexBuffer().QueueExecution(queue);
        vkCmdDrawIndexed(queue, buffers.indexBuffer.Size(), 1, 0, 0, 0);
    }

} // namespace Engine::Graphics