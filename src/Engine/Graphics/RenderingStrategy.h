#pragma once

#include "Graphics/CommandQueue.h"
#include "Graphics/Image.h"
#include "Graphics/RenderingRequest.h"
#include "Graphics/UniformBinder.h"
#include <optional>
#include <vector>

namespace Engine::Graphics {

class RenderingStrategy {
public:
  virtual ~RenderingStrategy() = default;
  // A rendering strategy should render to renderTarget and copy its depth buffer to depthTarget if depthTarget is not
  // null.
  virtual void RecordRenderingCommands(RenderingRequest const &request, UniformBinder &uniformBufferProvider,
                                       DescriptorAllocator &descriptorAllocator, DescriptorWriter &descriptorWriter,
                                       Image<2> &renderTarget, std::optional<Image<2>> &depthTarget,
                                       CommandRecorder const &recorder) = 0;
};

class BackgroundStrategy : public RenderingStrategy {
public:
  virtual ~BackgroundStrategy() = default;
  virtual void RecordRenderingCommands(Image<2> &renderTarget, CommandRecorder const &recorder) = 0;

  void RecordRenderingCommands(RenderingRequest const &request, UniformBinder &uniformBufferProvider,
                               DescriptorAllocator &descriptorAllocator, DescriptorWriter &descriptorWriter,
                               Image<2> &renderTarget, std::optional<Image<2>> &depthTarget,
                               CommandRecorder const &recorder) override {
    RecordRenderingCommands(renderTarget, recorder);
  }
};
} // namespace Engine::Graphics
