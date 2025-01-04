#pragma once

#include "Graphics/Buffer.h"
#include "Graphics/Camera.h"
#include "Graphics/DescriptorHandling.h"
#include "Graphics/DrawData.h"
#include "Graphics/GPUObjectManager.h"
#include "Graphics/InstanceManager.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/RenderingStrategy.h"
#include "vulkan/vulkan.h"
#include <array>
#include <vector>

namespace Engine::Graphics {

class MeshRenderer;

class Renderer {
  InstanceManager const *instanceManager;
  GPUObjectManager *gpuObjectManager;
  RenderingStrategy *renderingStrategy;

  struct FrameResources {
    CommandQueue commandQueue;
    DeletionQueue deletionQueue;
    VkSemaphore swapchainSemaphore;
    VkSemaphore renderSemaphore;
    VkFence renderFence;
    DescriptorAllocator descriptorAllocator;
    Buffer<DrawData> uniformBuffer;
  };

private:
  uint8_t currentBackgroundEffect = 1;

  static const uint32_t MAX_FRAME_OVERLAP = 3;

  VkQueue graphicsQueue;
  VkQueue presentQueue;

  VkSwapchainKHR swapchain;
  VkFormat swapchainFormat;
  VkExtent2D swapchainExtent;
  std::vector<Image<2>> swapchainImages;
  std::array<FrameResources, MAX_FRAME_OVERLAP> frameResources;
  uint32_t currentFrame = 0;

  Maths::Dimension2 windowDimension;
  Maths::Dimension2 renderBufferDimension{1600, 900};
  float renderScale = 1.0f;
  DescriptorAllocator descriptorAllocator;
  DescriptorLayoutBuilder descriptorLayoutBuilder;
  DescriptorWriter descriptorWriter;
  VkDescriptorSetLayout singleTextureDescriptorLayout;

  void CreateSwapchain();
  void DestroySwapchain();

  void CreateFrameResources(FrameResources &resources);
  void DestroyFrameResources(FrameResources &resources);

  VkFormat ChooseRenderBufferFormat();

  inline FrameResources const &CurrentResources() const { return frameResources[currentFrame % MAX_FRAME_OVERLAP]; }

public:
  Renderer(Maths::Dimension2 const &windowSize, InstanceManager const *instanceManager,
           GPUObjectManager *gpuObjectManager);
  ~Renderer();

  void DrawFrame(RenderingRequest const &request);

  inline void RecreateSwapchain() {
    instanceManager->WaitUntilDeviceIdle();
    DestroySwapchain();
    CreateSwapchain();
  }

  inline void SetRenderingStrategy(RenderingStrategy *newStrategy) { renderingStrategy = newStrategy; }
  inline void SetWindowSize(Maths::Dimension2 newSize) {
    windowDimension = newSize;
    RecreateSwapchain();
  }

  inline VkFormat GetSwapchainFormat() { return swapchainFormat; }
};

} // namespace Engine::Graphics
