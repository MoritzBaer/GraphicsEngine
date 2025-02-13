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
    DescriptorWriter descriptorWriter;
    Buffer<DrawData> uniformBuffer;
  };

  virtual FrameResources GetFrameResources() = 0;
  virtual Image2 &GetRenderTarget(bool &acquisitionSuccessful) = 0;
  virtual std::vector<Command const *> PrepareTargetForRendering() = 0;
  virtual std::vector<Command const *> PrepareTargetForDisplaying() = 0;
  virtual void DisplayRenderTarget() = 0;
};

inline void CreateFrameResources(RenderResourceProvider::FrameResources &resources,
                                 InstanceManager const *instanceManager,
                                 GPUObjectManager
#ifdef NDEBUG
                                 const
#endif
                                     *gpuObjectManager) {

  VkFenceCreateInfo fenceInfo = vkinit::FenceCreateInfo();

  VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

  instanceManager->CreateFence(&fenceInfo, &resources.renderFence);
  instanceManager->CreateSemaphore(&semaphoreInfo, &resources.renderSemaphore);
  instanceManager->CreateSemaphore(&semaphoreInfo, &resources.presentSemaphore);

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
  resources.uniformBuffer =
      gpuObjectManager->CreateBuffer<DrawData>(1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
}

inline void DestroyFrameResources(RenderResourceProvider::FrameResources &resources,
                                  InstanceManager const *instanceManager,
                                  GPUObjectManager
#ifdef NDEBUG
                                  const
#endif
                                      *gpuObjectManager) {
  gpuObjectManager->DestroyCommandQueue(resources.commandQueue);
  instanceManager->DestroySemaphore(resources.presentSemaphore);
  instanceManager->DestroySemaphore(resources.renderSemaphore);
  instanceManager->DestroyFence(resources.renderFence);
  resources.descriptorAllocator.ClearDescriptors();
  resources.descriptorAllocator.DestroyPools();
  resources.descriptorWriter.Clear();
  gpuObjectManager->DestroyBuffer(resources.uniformBuffer);
}

} // namespace Engine::Graphics
