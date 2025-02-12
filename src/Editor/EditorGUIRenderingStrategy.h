#pragma once

#include "Graphics/RenderingStrategy.h"

namespace Editor {

class EditorGUIRenderingStrategy : public Engine::Graphics::RenderingStrategy {
  Engine::Graphics::RenderingStrategy *subStrategy;
  Engine::Graphics::GPUObjectManager
#ifdef NDEBUG
      const
#endif
          *objectManager;
  Engine::Graphics::AllocatedImage2 renderBuffer;
  Engine::Maths::Dimension2 targetResolution;

public:
  void CreateRenderBuffer();
  void DestroyRenderBuffer();
  std::vector<Engine::Graphics::Command *> GetRenderingCommands(
      Engine::Graphics::RenderingRequest const &request, Engine::Maths::Dimension2 const &renderDimension,
      Engine::Graphics::Buffer<Engine::Graphics::DrawData> const &uniformBuffer,
      Engine::Graphics::DescriptorAllocator &descriptorAllocator, Engine::Graphics::DescriptorWriter &descriptorWriter,
      Engine::Graphics::Image<2> &renderTarget) override;

  EditorGUIRenderingStrategy(Engine::Graphics::GPUObjectManager
#ifdef NDEBUG
                             const
#endif
                                 *objectManager,
                             Engine::Graphics::RenderingStrategy *subStrategy)
      : subStrategy(subStrategy), objectManager(objectManager), targetResolution(1600, 900) {
    CreateRenderBuffer();
  }
  ~EditorGUIRenderingStrategy() { DestroyRenderBuffer(); }
};

} // namespace Editor