#include "Mesh.h"
#include "Renderer.h"

namespace Engine::Graphics
{
    class UnstageMeshCommand : public Command 
    {
        Buffer::BufferCopyCommand vertices;
        Buffer::BufferCopyCommand indices;
    public:
        UnstageMeshCommand(Buffer stagingBuffer, Buffer vertexBuffer, Buffer indexBuffer) 
            : vertices(stagingBuffer.CopyTo(vertexBuffer, vertexBuffer.Size())), indices(stagingBuffer.CopyTo(indexBuffer, indexBuffer.Size(), vertexBuffer.Size())) { }
        inline void QueueExecution(VkCommandBuffer const & queue) const {
            vertices.QueueExecution(queue);
            indices.QueueExecution(queue);
        }
    };

    void Mesh::Upload()
    {
        const size_t vertexBufferSize = vertices.size() * sizeof(VertexFormat);
        const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

        gpuBuffers.indexBuffer.Create(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        gpuBuffers.vertexBuffer.Create(vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        gpuBuffers.vertexBufferAddress = gpuBuffers.vertexBuffer.GetDeviceAddresss();

        Buffer stagingBuffer(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

        void * data = stagingBuffer.GetMappedData();
        memcpy(data, vertices.data(), vertexBufferSize);
        memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

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
        vkCmdDrawIndexed(queue, buffers.indexBuffer.Size() / sizeof(uint32_t), 1, 0, 0, 0);
    }

} // namespace Engine::Graphics