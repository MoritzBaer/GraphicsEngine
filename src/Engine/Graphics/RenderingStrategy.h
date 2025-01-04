#pragma once

#include "Graphics/CommandQueue.h"
#include "Graphics/Image.h"
#include "Graphics/RenderingRequest.h"
#include <vector>

namespace Engine::Graphics {

class RenderingStrategy {
public:
  virtual std::vector<Command *> GetRenderingCommands(RenderingRequest const &request,
                                                      Maths::Dimension2 const &renderDimension,
                                                      Buffer<DrawData> const &uniformBuffer,
                                                      DescriptorAllocator &descriptorAllocator,
                                                      DescriptorWriter &descriptorWriter, Image<2> &renderTarget) = 0;
};

class BackgroundStrategy : public RenderingStrategy {
public:
  virtual std::vector<Command *> GetRenderingCommands(Maths::Dimension2 const &renderDimension,
                                                      Image<2> &renderTarget) = 0;
  inline std::vector<Command *>
  GetRenderingCommands(RenderingRequest const &request, Maths::Dimension2 const &renderDimension,
                       Buffer<DrawData> const &uniformBuffer, DescriptorAllocator &descriptorAllocator,
                       DescriptorWriter &descriptorWriter, Image<2> &renderTarget) override {
    return GetRenderingCommands(renderDimension, renderTarget);
  }
};
} // namespace Engine::Graphics
