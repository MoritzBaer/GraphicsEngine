#pragma once

#include "Graphics/Buffer.h"
#include "Graphics/Camera.h"
#include "Graphics/ComputeEffect.h"
#include "Graphics/DescriptorHandling.h"
#include "Graphics/DrawData.h"
#include "Graphics/GPUObjectManager.h"
#include "Graphics/InstanceManager.h"
#include "Graphics/MeshRenderer.h"
#include "vulkan/vulkan.h"
#include <array>
#include <vector>

namespace Engine::Graphics {

class MeshRenderer;

class Renderer {
  InstanceManager const *instanceManager;
  GPUObjectManager *gpuObjectManager;

  struct FrameResources {
    CommandQueue commandQueue;
    DeletionQueue deletionQueue;
    VkSemaphore swapchainSemaphore;
    VkSemaphore renderSemaphore;
    VkFence renderFence;
    DescriptorAllocator descriptorAllocator;
    Buffer<DrawData> uniformBuffer;
  };

public:
  struct CompiledEffect {
    const char *name;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    ComputePushConstants data;
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
  bool renderBufferInitialized;
  struct {
    AllocatedImage<2> colourImage;
    AllocatedImage<2> depthImage;
  } renderBuffer;
  Maths::Dimension2 renderBufferDimension{1600, 900};
  float renderScale = 1.0f;
  DescriptorAllocator descriptorAllocator;
  DescriptorLayoutBuilder descriptorLayoutBuilder;
  DescriptorWriter descriptorWriter;
  VkDescriptorSet renderBufferDescriptors;
  VkDescriptorSetLayout renderBufferDescriptorLayout;
  VkDescriptorSetLayout singleTextureDescriptorLayout;

  std::vector<CompiledEffect> backgroundEffects;

  void CreateSwapchain();
  void DestroySwapchain();
  void InitDescriptors();
  void CompileBackgroundEffects(std::vector<ComputeEffect<ComputePushConstants>> const &uncompiledEffects);

  void RecreateRenderBuffer();
  void DestroyRenderBuffer();
  void CreateFrameResources(FrameResources &resources);
  void DestroyFrameResources(FrameResources &resources);

  VkFormat ChooseRenderBufferFormat();

  inline FrameResources const &CurrentResources() const { return frameResources[currentFrame % MAX_FRAME_OVERLAP]; }

public:
  Renderer(Maths::Dimension2 const &windowSize, InstanceManager const *instanceManager,
           GPUObjectManager *gpuObjectManager,
           std::vector<ComputeEffect<ComputePushConstants>> const &backgroundEffects);
  ~Renderer();

  void DrawFrame(Camera const *camera, SceneData const &sceneData,
                 std::span<MeshRenderer const *> const &objectsToDraw);

  inline void RecreateSwapchain() {
    instanceManager->WaitUntilDeviceIdle();
    DestroySwapchain();
    CreateSwapchain();
  }

  inline void SetWindowSize(Maths::Dimension2 newSize) {
    windowDimension = newSize;
    RecreateSwapchain();
  }

  void GetImGUISection();

  inline VkFormat GetSwapchainFormat() { return swapchainFormat; }
};

} // namespace Engine::Graphics
