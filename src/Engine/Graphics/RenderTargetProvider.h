#pragma once

#include "Image.h"

namespace Engine::Graphics {
struct RenderResourceProvider {

  struct FrameResources {
    CommandQueue commandQueue;
    VkSemaphore presentSemaphore;
    VkSemaphore renderSemaphore;
    VkFence renderFence;
    DescriptorAllocator descriptorAllocator;
    Buffer<DrawData> uniformBuffer;
  };

  virtual FrameResources GetFrameResources() = 0;
  virtual Image2 &GetRenderTarget(bool &acquisitionSuccessful) = 0;
  virtual std::vector<Command const *> PrepareTargetForRendering() = 0;
  virtual std::vector<Command const *> PrepareTargetForDisplaying() = 0;
  virtual void DisplayRenderTarget() = 0;
};

} // namespace Engine::Graphics
