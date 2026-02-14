#pragma once

#include "Image.h"
#include "UniformBinder.h"

namespace Engine::Graphics {
struct RenderResourceProvider {

  struct FrameResources {
    CommandQueue commandQueue;
    std::optional<VkSemaphore> renderSemaphore;
    VkFence renderFence;
    DescriptorAllocator descriptorAllocator;
    DescriptorWriter descriptorWriter;
    UniformBinder uniformBinder;
  };

  struct RenderTarget {
    std::optional<VkSemaphore> presentSemaphore;
    Image2 &target;
  };

  virtual FrameResources GetFrameResources() = 0;
  virtual std::optional<RenderTarget> GetRenderTarget() = 0;
  virtual std::vector<Command const *> PrepareTargetForRendering() = 0;
  virtual std::vector<Command const *> PrepareTargetForDisplaying() = 0;
  virtual void DisplayRenderTarget() = 0;
};

inline void CreateFrameResources(RenderResourceProvider::FrameResources &resources,
                                 InstanceManager const *instanceManager,
                                 GPUObjectManager RELEASE_CONST *gpuObjectManager) {
  VkFenceCreateInfo fenceInfo = vkinit::FenceCreateInfo();

  VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

  instanceManager->CreateFence(&fenceInfo, &resources.renderFence);
  VkSemaphore renderSemaphore;
  instanceManager->CreateSemaphore(&semaphoreInfo, &renderSemaphore);
  resources.renderSemaphore = renderSemaphore;

  resources.commandQueue = gpuObjectManager->CreateCommandQueue();

  std::vector<DescriptorAllocator::PoolSizeRatio> frame_sizes = {
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4},
  };

  resources.descriptorWriter = DescriptorWriter(instanceManager);
  resources.descriptorAllocator = DescriptorAllocator(instanceManager);
  resources.descriptorAllocator.InitPools(10, frame_sizes);
  resources.uniformBinder = UniformBinder(gpuObjectManager);
}

inline void DestroyFrameResources(RenderResourceProvider::FrameResources &resources,
                                  InstanceManager const *instanceManager,
                                  GPUObjectManager RELEASE_CONST *gpuObjectManager) {
  gpuObjectManager->DestroyCommandQueue(resources.commandQueue);
  if (resources.renderSemaphore.has_value()) {
    instanceManager->DestroySemaphore(resources.renderSemaphore.value());
  }
  instanceManager->DestroyFence(resources.renderFence);
  resources.descriptorAllocator.ClearDescriptors();
  resources.descriptorAllocator.DestroyPools();
  resources.descriptorWriter.Clear();
  resources.uniformBinder.Destroy();
}

} // namespace Engine::Graphics
