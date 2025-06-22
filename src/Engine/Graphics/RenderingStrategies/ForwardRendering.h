#pragma once

#include "Graphics/GPUObjectManager.h"
#include "Graphics/RenderingStrategy.h"

namespace Engine::Graphics::RenderingStrategies {

class ForwardRendering : public RenderingStrategy {
  GPUObjectManager *objectManager;
  BackgroundStrategy *backgroundStrategy;
  InstanceManager const *instanceManager;

  struct {
    Image2 colourImage;
    Image2 depthImage;
  } renderBuffer;

  VkFormat ChooseRenderBufferFormat();
  void CreateRenderBuffer(Dimension2 const &renderDimension);
  void DestroyRenderBuffer();

public:
  std::vector<Command const *> GetRenderingCommands(RenderingRequest const &request,
                                                    UniformBinder &uniformBufferProvider,
                                                    DescriptorAllocator &descriptorAllocator,
                                                    DescriptorWriter &descriptorWriter, Image<2> &renderTarget,
                                                    std::optional<Image<2>> &depthTarget) override;

  ForwardRendering(InstanceManager const *instanceManager, GPUObjectManager *objectManager,
                   BackgroundStrategy *backgroundStrategy)
      : objectManager(objectManager), instanceManager(instanceManager), backgroundStrategy(backgroundStrategy) {
    CreateRenderBuffer({1600, 900});
  }
  ~ForwardRendering() { DestroyRenderBuffer(); }
};

} // namespace Engine::Graphics::RenderingStrategies