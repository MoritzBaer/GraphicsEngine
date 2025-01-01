#pragma once

#include "Graphics/RenderingStrategy.h"

namespace Editor {
class DebugGUIRenderingStrategy : public Engine::Graphics::RenderingStrategy {
  Engine::Graphics::RenderingStrategy *renderingStrategy;

public:
  DebugGUIRenderingStrategy(Engine::Graphics::RenderingStrategy *renderingStrategy)
      : renderingStrategy(renderingStrategy) {}

  std::vector<Engine::Graphics::Command *>
  GetRenderingCommands(Engine::Graphics::RenderingRequest const &request, Engine::Graphics::Image<2> &colourImage,
                       Engine::Graphics::Image<2> &depthImage, Engine::Maths::Dimension2 const &renderDimension,
                       Engine::Graphics::Buffer<Engine::Graphics::DrawData> const &uniformBuffer,
                       Engine::Graphics::DescriptorAllocator &descriptorAllocator,
                       Engine::Graphics::DescriptorWriter &descriptorWriter,
                       Engine::Graphics::Image<2> &swapchainImage) override;
};
} // namespace Editor