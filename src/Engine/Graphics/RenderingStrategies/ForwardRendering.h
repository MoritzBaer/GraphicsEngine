#pragma once

#include "Graphics/CommandQueue.h"
#include "Graphics/GPUObjectManager.h"
#include "Graphics/RenderingStrategy.h"

namespace Engine::Graphics::RenderingStrategies {

class ForwardRendering : public RenderingStrategy {
  GPUObjectManager RELEASE_CONST *objectManager;
  BackgroundStrategy *backgroundStrategy;
  InstanceManager const *instanceManager;

  struct {
    Image2 colourImage;
    Image2 depthImage;
    VkFence renderFence;
  } renderBuffer;

  VkFormat ChooseRenderBufferFormat();
  void CreateRenderBuffer(Dimension2 const &renderDimension);
  void DestroyRenderBuffer();

public:
  void RecordRenderingCommands(RenderingRequest const &request,
                                                    UniformBinder &uniformBufferProvider,
                                                    DescriptorAllocator &descriptorAllocator,
                                                    DescriptorWriter &descriptorWriter, Image<2> &renderTarget,
                                                    std::optional<Image<2>> &depthTarget, CommandRecorder const & recorder) override;

  ForwardRendering(InstanceManager const *instanceManager, GPUObjectManager RELEASE_CONST *objectManager,
                   BackgroundStrategy *backgroundStrategy)
      : objectManager(objectManager), instanceManager(instanceManager), backgroundStrategy(backgroundStrategy) {
    CreateRenderBuffer({1600, 900});
  }
  ~ForwardRendering() { DestroyRenderBuffer(); }
};

} // namespace Engine::Graphics::RenderingStrategies