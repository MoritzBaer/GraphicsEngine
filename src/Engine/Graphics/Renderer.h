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
private:
  InstanceManager const *instanceManager;
  RenderingStrategy *renderingStrategy;
  RenderResourceProvider *renderResourceProvider;

  VkQueue graphicsQueue;

public:
  Renderer(InstanceManager const *instanceManager);
  ~Renderer();

  void DrawFrame(RenderingRequest const &request);

  inline void SetRenderResourceProvider(RenderResourceProvider *newProvider) { renderResourceProvider = newProvider; }
  inline void SetRenderingStrategy(RenderingStrategy *newStrategy) { renderingStrategy = newStrategy; }
};

} // namespace Engine::Graphics
