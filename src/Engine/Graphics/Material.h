#pragma once

#include "vulkan/vulkan.h"
#include "CommandQueue.h"
#include "UniformAggregate.h"
#include "Util/DeletionQueue.h"

namespace Engine::Graphics
{
    class Material : public Destroyable {
        VkPipeline pipeline;
        VkPipelineLayout layout;
    protected:
        Material(VkPipelineLayout layout, VkPipeline pipeline) : pipeline(pipeline), layout(layout) {}
        Material(Material const * other) : pipeline(other->pipeline), layout(other->layout) { }
    public: 
        Material() = delete;
        inline void Bind(VkCommandBuffer const & commandBuffer) const { vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline); } 
        inline VkPipelineLayout Layout() { return layout; }
        virtual void AppendData(UniformAggregate & aggregate) const = 0;
        void Destroy() const;
    };
} // namespace Engine::Graphics
