#pragma once

#include "Image.h"

namespace Engine::Graphics {
struct RenderResourceProvider {

  struct RenderResources {
    CommandQueue commandQueue;
    VkSemaphore presentSemaphore;
    VkSemaphore renderSemaphore;
    VkFence renderFence;
    DescriptorAllocator descriptorAllocator;
    Buffer<DrawData> uniformBuffer;
    Image2 *renderTarget;
  };

  virtual RenderResources GetRenderResources() = 0;
  virtual std::vector<Command const *> PrepareTargetForRendering() = 0;
  virtual std::vector<Command const *> PrepareTargetForDisplaying() = 0;
  virtual void DisplayRenderTarget() = 0;
};

} // namespace Engine::Graphics
