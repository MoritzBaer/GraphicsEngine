#pragma once

#include "Graphics/CommandQueue.h"
#include "Graphics/Image.h"
#include "Graphics/RenderingRequest.h"
#include <vector>

namespace Engine::Graphics {
class RenderingStrategy {
public:
  virtual std::vector<Command *> GetRenderingCommands(RenderingRequest const &request, Image<2> &colourImage,
                                                      Image<2> &depthImage, Maths::Dimension2 const &renderDimension,
                                                      Buffer<DrawData> const &uniformBuffer,
                                                      DescriptorAllocator &descriptorAllocator,
                                                      DescriptorWriter &descriptorWriter, Image<2> &swapchainImage) = 0;
};
} // namespace Engine::Graphics
