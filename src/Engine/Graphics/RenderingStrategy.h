#pragma once

#include "Graphics/CommandQueue.h"
#include "Graphics/Image.h"
#include "Graphics/RenderBufferPool.h"
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
                                       RenderBuffer renderBuffer, CommandRecorder const &recorder) = 0;
};

class BackgroundStrategy : public RenderingStrategy {
public:
  virtual ~BackgroundStrategy() = default;
  virtual void RecordRenderingCommands(DescriptorAllocator &descriptorAllocator, DescriptorWriter &descriptorWriter, RenderBuffer renderBuffer, CommandRecorder const &recorder) = 0;

  void RecordRenderingCommands(RenderingRequest const &request, UniformBinder &uniformBufferProvider,
                               DescriptorAllocator &descriptorAllocator, DescriptorWriter &descriptorWriter,
                               RenderBuffer renderBuffer, CommandRecorder const &recorder) override {
    RecordRenderingCommands(descriptorAllocator, descriptorWriter, renderBuffer, recorder);
  }
};
} // namespace Engine::Graphics
