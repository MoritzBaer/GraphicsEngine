#pragma once

#include "Buffer.h"
#include "Camera.h"
#include "CommandQueue.h"
#include "DescriptorHandling.h"
#include "DrawData.h"
#include "Image.h"
#include "Maths/Dimension.h"
#include "Shader.h"
#include "Util/DeletionQueue.h"
#include "Util/Macros.h"
#include "vulkan/vulkan.h"
#include <array>
#include <vector>

namespace Engine::Graphics {

class MeshRenderer;

class Renderer {
  _SINGLETON(Renderer, Maths::Dimension2 windowSize)

  struct FrameResources : public Initializable {
    CommandQueue commandQueue;
    DeletionQueue deletionQueue;
    VkSemaphore swapchainSemaphore;
    VkSemaphore renderSemaphore;
    VkFence renderFence;
    DescriptorAllocator descriptorAllocator;
    Buffer<DrawData> uniformBuffer;

    void Create() override;
    void Destroy() override;
  };

  struct ImmediateSubmitResources : public ConstInitializable {
    CommandQueue commandQueue;
    VkFence fence;

    void Create();
    void Destroy() const;
  } immediateResources;

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
  struct : public ConstDestroyable {
    AllocatedImage<2> colourImage;
    AllocatedImage<2> depthImage;
    inline void Destroy() const {
      colourImage.Destroy();
      depthImage.Destroy();
    }
  } renderBuffer;
  Maths::Dimension2 renderBufferDimension{1600, 900};
  float renderScale = 1.0f;
  DescriptorAllocator descriptorAllocator;
  VkDescriptorSet renderBufferDescriptors;
  VkDescriptorSetLayout renderBufferDescriptorLayout;
  VkDescriptorSetLayout singleTextureDescriptorLayout;

  void CreateSwapchain();
  void DestroySwapchain();
  void InitDescriptors();
  void InitPipelines();
  void InitBackgroundPipeline();

  void Draw(Camera const *camera, SceneData const &sceneData, std::span<MeshRenderer const *> const &objectsToDraw);
  void RecreateRenderBuffer();

  inline FrameResources const &CurrentResources() const { return frameResources[currentFrame % MAX_FRAME_OVERLAP]; }

public:
  // Signature is likely to change (for example a list of render objects will have to be passed somehow)
  static inline void DrawFrame(Camera const *camera, SceneData const &sceneData,
                               std::span<MeshRenderer const *> const &objectsToDraw) {
    instance->Draw(camera, sceneData, objectsToDraw);
  }

  static inline void RecreateSwapchain() {
    InstanceManager::WaitUntilDeviceIdle();
    instance->DestroySwapchain();
    instance->CreateSwapchain();
  }

  static inline void SetWindowSize(Maths::Dimension2 newSize) {
    instance->windowDimension = newSize;
    RecreateSwapchain();
  }

  static void ImmediateSubmit(Command *command);

  static void GetImGUISection();

  static inline VkFormat GetSwapchainFormat() { return instance->swapchainFormat; }
};

} // namespace Engine::Graphics
