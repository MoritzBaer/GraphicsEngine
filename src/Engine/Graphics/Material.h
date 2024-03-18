#pragma once

#include "vulkan/vulkan.h"
#include "CommandQueue.h"
#include "UniformAggregate.h"

namespace Engine::Graphics
{
    class Material {
        VkPipeline pipeline;
        VkPipelineLayout layout;
    protected:
        Material(VkPipelineLayout layout, VkPipeline pipeline) : pipeline(pipeline), layout(layout) {}
    public: 
        Material() = delete;
        ~Material();
        inline void Bind(VkCommandBuffer const & commandBuffer) const { vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline); } 
        inline VkPipelineLayout Layout() { return layout; }
        virtual void AppendData(UniformAggregate & aggregate) const = 0;

    };
} // namespace Engine::Graphics
