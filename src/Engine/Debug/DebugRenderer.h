#pragma once

#include "AssetManager.h"
#include "Graphics/BufferedRenderer.h"
#include "Maths/Matrix.h"
#include <cstddef>

namespace Engine::Graphics {
class Pipeline;
class GPUObjectManager;
} // namespace Engine::Graphics

namespace Engine {
class AssetManager;
}

using namespace Engine::Graphics;

namespace Engine::Debug {

struct DebugPoint {
  Maths::Vector3 position;
  Maths::Vector3 color;
};

struct DebugLine {
  Maths::Vector3 start;
  Maths::Vector3 end;
  Maths::Vector3 color;
};

class DebugRenderer {
public:
  struct Uniform {
    Maths::Matrix4 view;
    Maths::Matrix4 projection;
  };

private:
  BufferedRenderer<DebugPoint, Uniform> pointRenderer;
  BufferedRenderer<DebugLine, Uniform> lineRenderer;

  class DebugMaterial;

  class DepthBufferExtractor : public RenderTargetProvider {
    DebugMaterial *material;
    std::vector<Texture2D> textureDump;

  public:
    DepthBufferExtractor(GPUObjectManager RELEASE_CONST *gpuObjectManager)
        : RenderTargetProvider(gpuObjectManager), material(nullptr), textureDump() {}

    inline void SetMaterial(DebugMaterial *material) { this->material = material; }

    inline std::tuple<Image2, Image2> GetRenderTarget(Image2 &givenRenderTarget,
                                                      std::optional<Image2> &givenDepthTarget,
                                                      std::vector<Command const *> &previousCommands) override;
  } bufferExtractor;

  class DebugMaterial : public Material {
    Texture2D mainPassDepthBuffer;
    friend class DepthBufferExtractor;

  public:
    DebugMaterial(Pipeline const *pipeline) : Material(pipeline), mainPassDepthBuffer() {}

    inline void AppendData(PushConstantsAggregate &aggregate) const override {}
    inline void BindDescriptors(std::vector<VkDescriptorSet> &descriptorSets, DescriptorAllocator &descriptorAllocator,
                                DescriptorWriter &writer, UniformBinding const &uniform) const override {
      descriptorSets.push_back(pipeline->AllocateForLayout(0, descriptorAllocator));
      uniform.WriteToDescriptorSet(writer, descriptorSets.back(), 0);
      descriptorSets.push_back(pipeline->AllocateForLayout(1, descriptorAllocator));
      mainPassDepthBuffer.UpdateDescriptors(writer, descriptorSets.back(), 0);
    }
  };

public:
  DebugRenderer(GPUObjectManager RELEASE_CONST *gpuObjectManager, InstanceManager const *instanceManager)
      : pointRenderer(instanceManager, gpuObjectManager, &bufferExtractor),
        lineRenderer(instanceManager, gpuObjectManager, &bufferExtractor), bufferExtractor(gpuObjectManager) {}

  void AddPoint(Maths::Vector3 position, Maths::Vector3 color) { pointRenderer.AddToBuffer({position, color}); }
  void AddLine(Maths::Vector3 start, Maths::Vector3 end, Maths::Vector3 color) {
    lineRenderer.AddToBuffer({start, end, color});
  }

  void Clear() {
    pointRenderer.ClearBuffer();
    lineRenderer.ClearBuffer();
  }

  void InitPipelines(AssetManager &assetManager) {
    auto pointMat = new DebugMaterial(assetManager.LoadAsset<Pipeline *>("debug_points"));
    pointRenderer.SetMaterial(pointMat);
    bufferExtractor.SetMaterial(pointMat);
  }

  Graphics::RenderingStrategy *Wrap(Graphics::RenderingStrategy *strategy) {
    return pointRenderer.WrapWithBufferedStrategy(lineRenderer.WrapWithBufferedStrategy(strategy));
  }
};

inline std::tuple<Image2, Image2>
DebugRenderer::DepthBufferExtractor::GetRenderTarget(Image2 &givenRenderTarget, std::optional<Image2> &givenDepthTarget,
                                                     std::vector<Command const *> &previousCommands) {
  for (auto const &texture : textureDump) {
    gpuObjectManager->DestroyTexture(texture);
  }
  textureDump.clear();

  if (givenDepthTarget.has_value()) {
    if (material->mainPassDepthBuffer.GetExtent() != givenDepthTarget->GetExtent()) {
      // Create new texture of fitting dimension
      textureDump.push_back(material->mainPassDepthBuffer);
      material->mainPassDepthBuffer = gpuObjectManager->CreateTexture(
          givenDepthTarget->GetExtent(), VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_FORMAT_D32_SFLOAT, false,
          VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
          VK_IMAGE_ASPECT_DEPTH_BIT);
    }
    previousCommands.push_back(givenDepthTarget->Transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL));
    previousCommands.push_back(material->mainPassDepthBuffer.Transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));
    previousCommands.push_back(givenDepthTarget->BlitTo(material->mainPassDepthBuffer, VK_FILTER_NEAREST));
    previousCommands.push_back(material->mainPassDepthBuffer.Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
    previousCommands.push_back(givenDepthTarget->Transition(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
  }
  return RenderTargetProvider::GetRenderTarget(givenRenderTarget, givenDepthTarget, previousCommands);
}

} // namespace Engine::Debug