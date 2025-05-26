#pragma once

#include "AssetManager.h"
#include "Graphics/BufferedRenderer.h"
#include "Maths/Matrix.h"

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

  class DebugMaterial : public Material {
  public:
    DebugMaterial(Pipeline const *pipeline) : Material(pipeline) {}

    inline void AppendData(PushConstantsAggregate &aggregate) const override {}
    inline void BindDescriptors(std::vector<VkDescriptorSet> &descriptorSets, DescriptorAllocator &descriptorAllocator,
                     DescriptorWriter &writer, UniformBinding const &uniform) const override {
      descriptorSets.push_back(pipeline->AllocateForLayout(0, descriptorAllocator));
      uniform.WriteToDescriptorSet(writer, descriptorSets.back(), 0);
    }
  };

public:
  DebugRenderer(GPUObjectManager *gpuObjectManager, InstanceManager const *instanceManager)
      : pointRenderer(instanceManager, gpuObjectManager), lineRenderer(instanceManager, gpuObjectManager) {}

  void AddPoint(Maths::Vector3 position, Maths::Vector3 color) { pointRenderer.AddToBuffer({position, color}); }
  void AddLine(Maths::Vector3 start, Maths::Vector3 end, Maths::Vector3 color) {
    lineRenderer.AddToBuffer({start, end, color});
  }

  void Clear() {
    pointRenderer.ClearBuffer();
    lineRenderer.ClearBuffer();
  }

  void InitPipelines(AssetManager &assetManager) {
    pointRenderer.SetMaterial(new DebugMaterial(assetManager.LoadAsset<Pipeline *>("debug_points")));
  }

  Graphics::RenderingStrategy *Wrap(Graphics::RenderingStrategy *strategy) {
    return pointRenderer.WrapWithBufferedStrategy(lineRenderer.WrapWithBufferedStrategy(strategy));
  }
};

} // namespace Engine::Debug