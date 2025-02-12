#pragma once

#include "Graphics/Buffer.h"
#include "Graphics/Camera.h"
#include "Graphics/DescriptorHandling.h"
#include "Graphics/DrawData.h"
#include "Graphics/GPUObjectManager.h"
#include "Graphics/InstanceManager.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/RenderTargetProvider.h"
#include "Graphics/RenderingStrategy.h"
#include "vulkan/vulkan.h"
#include <array>
#include <vector>

namespace Engine::Graphics {

class MeshRenderer;

class Renderer {
  InstanceManager const *instanceManager;
  GPUObjectManager const *gpuObjectManager;
  RenderingStrategy *renderingStrategy;
  RenderResourceProvider *frameResourceProvider;

private:
  uint8_t currentBackgroundEffect = 1;

  static const uint32_t MAX_FRAME_OVERLAP = 3;

  VkQueue graphicsQueue;

  Maths::Dimension2 windowDimension;
  Maths::Dimension2 renderBufferDimension{1600, 900};
  float renderScale = 1.0f;
  DescriptorAllocator descriptorAllocator;
  DescriptorLayoutBuilder descriptorLayoutBuilder;
  DescriptorWriter descriptorWriter;
  VkDescriptorSetLayout singleTextureDescriptorLayout;

public:
  Renderer(InstanceManager const *instanceManager, GPUObjectManager const *gpuObjectManager);
  ~Renderer();

  void DrawFrame(RenderingRequest const &request);

  inline void SetFrameResourceProvider(RenderResourceProvider *newProvider) { frameResourceProvider = newProvider; }
  inline void SetRenderingStrategy(RenderingStrategy *newStrategy) { renderingStrategy = newStrategy; }
};

} // namespace Engine::Graphics
