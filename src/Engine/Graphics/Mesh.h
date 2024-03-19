#pragma once

#include "Maths/Matrix.h"
#include "Buffer.h"
#include "Renderer.h"
#include <vector>
#include "CommandQueue.h"
#include "Util/DeletionQueue.h"
#include "UniformAggregate.h"

namespace Engine::Graphics
{
    class Mesh : public Destroyable
    {
        struct VertexFormat;

        struct GPUBuffers
        {
            Buffer<VertexFormat> vertexBuffer;   
            Buffer<uint32_t> indexBuffer;
            VkDeviceAddress vertexBufferAddress;
        } gpuBuffers;
        
        struct GPUPushConstants
        {
            VkDeviceAddress vertexBufferAddress;  
        } pushConstants;

        friend class Renderer;

    public:
        struct VertexFormat
        {
            Maths::Vector3 position;
            float uv_x;
            Maths::Vector3 normal;
            float uv_y;
            Maths::Vector4 colour;
        };

        std::vector<VertexFormat> vertices;
        std::vector<uint32_t> indices;

        void Upload();
        void Destroy() const;

        inline void BindAndDraw(VkCommandBuffer const & commandBuffer) const { 
            gpuBuffers.indexBuffer.BindAsIndexBuffer(commandBuffer);
            vkCmdDrawIndexed(commandBuffer, gpuBuffers.indexBuffer.Size(), 1, 0, 0, 0);
        }
        inline void AppendData(UniformAggregate & aggregate) const { aggregate.PushData(&pushConstants); }
    };
    
} // namespace Engine::Graphics
