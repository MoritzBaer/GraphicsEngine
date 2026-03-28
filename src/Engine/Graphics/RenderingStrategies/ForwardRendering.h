#pragma once

#include "Graphics/CommandQueue.h"
#include "Graphics/GPUObjectManager.h"
#include "Graphics/RenderBufferPool.h"
#include "Graphics/RenderingStrategy.h"

namespace Engine::Graphics::RenderingStrategies {

class ForwardRendering : public RenderingStrategy {
  GPUObjectManager RELEASE_CONST *objectManager;
  BackgroundStrategy *backgroundStrategy;
  InstanceManager const *instanceManager;

  VkFormat ChooseRenderBufferFormat();

public:
  void RecordRenderingCommands(RenderingRequest const &request, UniformBinder &uniformBufferProvider,
                               DescriptorAllocator &descriptorAllocator, DescriptorWriter &descriptorWriter,
                               RenderBuffer renderBuffer, CommandRecorder const &recorder) override;

  ForwardRendering(InstanceManager const *instanceManager, GPUObjectManager RELEASE_CONST *objectManager,
                   BackgroundStrategy *backgroundStrategy)
      : objectManager(objectManager), instanceManager(instanceManager), backgroundStrategy(backgroundStrategy) {}
  ~ForwardRendering() {}
};

} // namespace Engine::Graphics::RenderingStrategies