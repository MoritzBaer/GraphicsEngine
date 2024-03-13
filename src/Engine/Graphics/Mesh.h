#pragma once

#include "Maths/Matrix.h"
#include "Buffer.h"
#include "Renderer.h"
#include <vector>
#include "CommandQueue.h"
#include "Util/DeletionQueue.h"

namespace Engine::Graphics
{
    class Mesh : public Destroyable
    {
        struct GPUMeshBuffers
        {
            Buffer vertexBuffer;   
            Buffer indexBuffer;
            VkDeviceAddress vertexBufferAddress;
        } gpuBuffers;
        
        struct GPUPushConstants
        {
            Maths::Matrix4 modelMatrix;
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
        void Destroy();

        inline class DrawMeshCommand : public Command {
            Mesh::GPUMeshBuffers buffers;
            VkPipelineLayout layout;
        public:
            void QueueExecution(VkCommandBuffer const & queue) const;
            DrawMeshCommand(Mesh::GPUMeshBuffers const & buffers, VkPipelineLayout const & pipelineLayout) : buffers(buffers), layout(pipelineLayout) { }
        } Draw(VkPipelineLayout const & pipelineLayout) const { return DrawMeshCommand(gpuBuffers, pipelineLayout); }
    };
    
} // namespace Engine::Graphics
