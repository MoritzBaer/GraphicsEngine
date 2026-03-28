#pragma once

#include "AssetManager.h"
#include "Graphics/BufferedRenderer.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/Material.h"
#include "Graphics/RenderingStrategy.h"
#include "Maths/Matrix.h"
#include <optional>
#include <span>
#include <vector>
#include <vulkan/vulkan_core.h>

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
  class DepthBufferReadingMaterial : public Material {
    Texture2D mainPassDepthBuffer;
    friend class DepthBufferExtractor;

  public:
    DepthBufferReadingMaterial(Pipeline const *pipeline) : Material(pipeline), mainPassDepthBuffer() {}

    inline void AppendData(PushConstantsAggregate &aggregate) const override {}
    inline std::vector<VkDescriptorSet> WriteDescriptors(DescriptorAllocator &descriptorAllocator,
                                                         DescriptorWriter &writer,
                                                         UniformBinding const &uniform) const override {
      std::vector<VkDescriptorSet> descriptorSets{};
      descriptorSets.push_back(pipeline->AllocateForLayout(0, descriptorAllocator));
      uniform.WriteToDescriptorSet(writer, descriptorSets.back(), 0);
      descriptorSets.push_back(pipeline->AllocateForLayout(1, descriptorAllocator));
      mainPassDepthBuffer.UpdateDescriptors(writer, descriptorSets.back(), 0);
      return descriptorSets;
    }
  };

  BufferedRenderer<DebugPoint, Uniform> pointRenderer;
  BufferedRenderer<DebugLine, Uniform> lineRenderer;

  std::vector<DepthBufferReadingMaterial *> materials;

public:
  DebugRenderer(GPUObjectManager RELEASE_CONST *gpuObjectManager, InstanceManager const *instanceManager)
      : pointRenderer(instanceManager, gpuObjectManager), lineRenderer(instanceManager, gpuObjectManager) {}

  void AddPoint(Maths::Vector3 position, Maths::Vector3 color) { pointRenderer.AddToBuffer({position, color}); }
  void AddLine(Maths::Vector3 start, Maths::Vector3 end, Maths::Vector3 color) {
    lineRenderer.AddToBuffer({start, end, color});
  }

  inline void BeginFrame() {
    pointRenderer.ClearBuffer();
    lineRenderer.ClearBuffer();
  }

  void InitPipelines(AssetManager &assetManager) {
    auto pointMat = new DepthBufferReadingMaterial(assetManager.LoadAsset<Pipeline *>("debug_points"));
    auto lineMat = new DepthBufferReadingMaterial(assetManager.LoadAsset<Pipeline *>("debug_lines"));
    pointRenderer.SetMaterial(pointMat);
    materials.push_back(pointMat);
    lineRenderer.SetMaterial(lineMat);
    materials.push_back(lineMat);
  }

  Graphics::RenderingStrategy *Wrap(Graphics::RenderingStrategy *strategy) {
    return pointRenderer.WrapWithBufferedStrategy(lineRenderer.WrapWithBufferedStrategy(strategy));
  }
};

// TODO: Get texture from depth image if present, otherwise pass single-pixel white
class DepthBufferExtractor : public RenderingStrategy {
  RenderingStrategy *mainStrategy;
  std::vector<Material *> depthMaterials;

public:
  inline void RecordRenderingCommands(RenderingRequest const &request, UniformBinder &uniformBufferProvider,
                                      DescriptorAllocator &descriptorAllocator, DescriptorWriter &descriptorWriter,
                                      RenderBuffer renderBuffer, CommandRecorder const &recorder) override {
    mainStrategy->RecordRenderingCommands(request, uniformBufferProvider, descriptorAllocator, descriptorWriter,
                                          renderBuffer, recorder);

    if (renderBuffer.DepthBufferInUse()) {

      recorder.RecordTransition(renderBuffer.depthImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
      for (auto &material : depthMaterials) {
        // Set depth texture for material
      }
    }
  }

  DepthBufferExtractor(RenderingStrategy *mainStrategy) : mainStrategy(mainStrategy) {}
};

} // namespace Engine::Debug