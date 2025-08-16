#pragma once

#include "AssetManager.h"
#include "Graphics/BufferedRenderer.h"
#include "Maths/Matrix.h"
#include <optional>

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
  class DepthBufferReadingMaterial;

  class DepthBufferExtractor : public RenderTargetProvider {
    std::vector<DepthBufferReadingMaterial *> materials;
    Image2 emptyDepthBuffer; // Needed in case the wrapped strategy does not produce a depth buffer
    inline static constexpr const float one = 1;
    Image2 depthBuffer;
    std::vector<Texture2D> textureDump;
    std::vector<Image2> depthBufferDump;
    bool doExtract = true;

  public:
    DepthBufferExtractor(GPUObjectManager RELEASE_CONST *gpuObjectManager)
        : RenderTargetProvider(gpuObjectManager), materials(), depthBuffer(), depthBufferDump(), textureDump(),
          emptyDepthBuffer(gpuObjectManager->CreateTexture(Dimension2(1, 1), &one, VK_FILTER_NEAREST, VK_FILTER_NEAREST,
                                                           VK_FORMAT_D32_SFLOAT, false, VK_SAMPLE_COUNT_1_BIT,
                                                           VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                           VK_IMAGE_ASPECT_DEPTH_BIT)) {}

    inline void AddMaterial(DepthBufferReadingMaterial *material) { materials.push_back(material); }

    inline std::tuple<Image2, Image2, VkAttachmentLoadOp>
    GetRenderTarget(Image2 &givenRenderTarget, std::optional<Image2> &givenDepthTarget,
                    std::vector<Command const *> &previousCommands) override;

    inline std::vector<Command *> GetTargetSwapCommands(Image2 &givenRenderTarget,
                                                        std::optional<Image2> &givenDepthTarget) {
      givenDepthTarget.emplace(depthBuffer);
      return {};
    }

    inline void BeginFrame();
  } bufferExtractor;

  class DepthBufferReadingMaterial : public Material {
    Texture2D mainPassDepthBuffer;
    friend class DepthBufferExtractor;

  public:
    DepthBufferReadingMaterial(Pipeline const *pipeline) : Material(pipeline), mainPassDepthBuffer() {}

    inline void AppendData(PushConstantsAggregate &aggregate) const override {}
    inline void BindDescriptors(std::vector<VkDescriptorSet> &descriptorSets, DescriptorAllocator &descriptorAllocator,
                                DescriptorWriter &writer, UniformBinding const &uniform) const override {
      descriptorSets.push_back(pipeline->AllocateForLayout(0, descriptorAllocator));
      uniform.WriteToDescriptorSet(writer, descriptorSets.back(), 0);
      descriptorSets.push_back(pipeline->AllocateForLayout(1, descriptorAllocator));
      mainPassDepthBuffer.UpdateDescriptors(writer, descriptorSets.back(), 0);
    }
  };

  BufferedRenderer<DebugPoint, Uniform> pointRenderer;
  BufferedRenderer<DebugLine, Uniform> lineRenderer;

public:
  DebugRenderer(GPUObjectManager RELEASE_CONST *gpuObjectManager, InstanceManager const *instanceManager)
      : bufferExtractor(gpuObjectManager), pointRenderer(instanceManager, gpuObjectManager, &bufferExtractor),
        lineRenderer(instanceManager, gpuObjectManager, &bufferExtractor) {}

  void AddPoint(Maths::Vector3 position, Maths::Vector3 color) { pointRenderer.AddToBuffer({position, color}); }
  void AddLine(Maths::Vector3 start, Maths::Vector3 end, Maths::Vector3 color) {
    lineRenderer.AddToBuffer({start, end, color});
  }

  inline void BeginFrame() {
    Clear();
    bufferExtractor.BeginFrame();
  }

  inline void Clear() {
    pointRenderer.ClearBuffer();
    lineRenderer.ClearBuffer();
  }

  void InitPipelines(AssetManager &assetManager) {
    auto pointMat = new DepthBufferReadingMaterial(assetManager.LoadAsset<Pipeline *>("debug_points"));
    auto lineMat = new DepthBufferReadingMaterial(assetManager.LoadAsset<Pipeline *>("debug_lines"));
    pointRenderer.SetMaterial(pointMat);
    lineRenderer.SetMaterial(lineMat);
    bufferExtractor.AddMaterial(pointMat);
    bufferExtractor.AddMaterial(lineMat);
  }

  Graphics::RenderingStrategy *Wrap(Graphics::RenderingStrategy *strategy) {
    return pointRenderer.WrapWithBufferedStrategy(lineRenderer.WrapWithBufferedStrategy(strategy));
  }
};

inline void DebugRenderer::DepthBufferExtractor::BeginFrame() {
  doExtract = true;
  for (auto const &texture : textureDump) {
    gpuObjectManager->DestroyTexture(texture);
  }
  textureDump.clear();
  for (auto const &db : depthBufferDump) {
    gpuObjectManager->DestroyImage(db);
  }
  depthBufferDump.clear();
  doExtract = true;
}

inline std::tuple<Image2, Image2, VkAttachmentLoadOp>
DebugRenderer::DepthBufferExtractor::GetRenderTarget(Image2 &givenRenderTarget, std::optional<Image2> &givenDepthTarget,
                                                     std::vector<Command const *> &previousCommands) {
  if (!doExtract || materials.empty()) {
    return RenderTargetProvider::GetRenderTarget(givenRenderTarget, givenDepthTarget, previousCommands);
  }

  doExtract = false;

  Image2 *depthSource = givenDepthTarget ? &givenDepthTarget.value() : &emptyDepthBuffer;

  if (depthBuffer.GetExtent() != depthSource->GetExtent()) {
    depthBufferDump.push_back(depthBuffer);
    depthBuffer = gpuObjectManager->CreateDepthBuffer(depthSource->GetExtent());
  }

  previousCommands.push_back(depthSource->Transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL));
  for (auto &material : materials) {
    if (material->mainPassDepthBuffer.GetExtent() != depthSource->GetExtent()) {
      // Create new texture of fitting dimension
      textureDump.push_back(material->mainPassDepthBuffer);
      material->mainPassDepthBuffer = gpuObjectManager->CreateTexture(
          depthSource->GetExtent(), VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_FORMAT_D32_SFLOAT, false,
          VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
          VK_IMAGE_ASPECT_DEPTH_BIT);
    }
    previousCommands.push_back(material->mainPassDepthBuffer.Transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));
    previousCommands.push_back(depthSource->BlitTo(material->mainPassDepthBuffer, VK_FILTER_NEAREST));
    previousCommands.push_back(material->mainPassDepthBuffer.Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
  }
  previousCommands.push_back(depthSource->Transition(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));

  return {givenRenderTarget, depthBuffer, VK_ATTACHMENT_LOAD_OP_CLEAR};
}

} // namespace Engine::Debug