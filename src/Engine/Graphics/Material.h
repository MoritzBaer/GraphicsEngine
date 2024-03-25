#pragma once

#include "UniformAggregate.h"
#include "Util/DeletionQueue.h"
#include "vulkan/vulkan.h"

namespace Engine::Graphics {

class Pipeline : public Destroyable {
  VkPipeline pipeline;
  VkPipelineLayout layout;

public:
  Pipeline(VkPipelineLayout layout, VkPipeline pipeline) : pipeline(pipeline), layout(layout) {}
  Pipeline(Pipeline const *other) : pipeline(other->pipeline), layout(other->layout) {}
  Pipeline() = delete;
  inline void Bind(VkCommandBuffer const &commandBuffer) const {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  }
  inline VkPipelineLayout Layout() const { return layout; }
  void Destroy() const;
};

class Material {
protected:
  Pipeline const *pipeline;

public:
  Material(Material const *other) : pipeline(other->pipeline) {}
  Material(Pipeline const *pipeline) : pipeline(pipeline) {}
  virtual void AppendData(UniformAggregate &aggregate) const = 0;
  inline Pipeline const *GetPipeline() const { return pipeline; }
};
} // namespace Engine::Graphics
