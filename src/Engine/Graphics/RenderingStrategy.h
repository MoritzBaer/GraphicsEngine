#pragma once

#include "Graphics/CommandQueue.h"
#include "Graphics/Image.h"
#include "Graphics/RenderingRequest.h"
#include "Graphics/UniformBinder.h"
#include <vector>

namespace Engine::Graphics {

class RenderingStrategy {
public:
  virtual ~RenderingStrategy() = default;
  // A rendering strategy should render to renderTarget and copy its depth buffer to depthTarget if depthTarget is not null.
  virtual std::vector<Command *> GetRenderingCommands(RenderingRequest const &request,
                                                      UniformBinder &uniformBufferProvider,
                                                      DescriptorAllocator &descriptorAllocator,
                                                      DescriptorWriter &descriptorWriter, Image<2> &renderTarget, Image<2> *depthTarget = nullptr) = 0;
};

class BackgroundStrategy : public RenderingStrategy {
public:
  virtual ~BackgroundStrategy() = default;
  virtual std::vector<Command *> GetRenderingCommands(Image<2> &renderTarget) = 0;
  inline std::vector<Command *> GetRenderingCommands(RenderingRequest const &request,
                                                     UniformBinder &uniformBufferProvider,
                                                     DescriptorAllocator &descriptorAllocator,
                                                     DescriptorWriter &descriptorWriter,
                                                     Image<2> &renderTarget, Image<2> *depthTarget) override {
    return GetRenderingCommands(renderTarget);
  }
};
} // namespace Engine::Graphics
