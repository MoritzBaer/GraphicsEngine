#pragma once

#include "Graphics/RenderingStrategy.h"

namespace Engine::Graphics::RenderingStrategies {

class ForwardRendering : public RenderingStrategy {
  std::vector<Command *> GetRenderingCommands(RenderingRequest const &request, Image<2> &colourImage,
                                              Image<2> &depthImage, Maths::Dimension2 const &renderDimension,
                                              Buffer<DrawData> const &uniformBuffer,
                                              DescriptorAllocator &descriptorAllocator,
                                              DescriptorWriter &descriptorWriter, Image<2> &swapchainImage) override;
};

} // namespace Engine::Graphics::RenderingStrategies