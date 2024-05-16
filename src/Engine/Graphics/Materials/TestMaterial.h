#pragma once

#include "Graphics/Material.h"
#include "Graphics/Texture.h"
#include "Maths/Dimension.h"

namespace Engine::Graphics::Materials {

class TestMaterial : public Material {

public:
  Texture2D texture;
  Maths::Vector3 colour;
  // TestMaterial(VkPipelineLayout l, VkPipeline p) { pipeline = new Pipeline(l, p)}
  TestMaterial(Material const *other) : Material(other) {
    if (TestMaterial const *testMat = dynamic_cast<TestMaterial const *>(other)) {
      colour = testMat->colour;
      texture = testMat->texture;
    } else {
      colour = {1, 1, 1};
      std::vector black = {0x000000FF};
      texture = Texture2D(Maths::Dimension2(1, 1), black.data());
    }
  }
  TestMaterial(Pipeline const *pipeline, Maths::Vector3 const &colour, Texture2D texture)
      : Material(pipeline), colour(colour), texture(texture) {}
  inline void AppendData(PushConstantsAggregate &aggregate) const { aggregate.PushData(&colour); }
  inline void Bind(VkCommandBuffer const &commandBuffer, DescriptorAllocator &descriptorAllocator,
                   Buffer<DrawData> const &drawData) const override {
    Material::Bind(commandBuffer, descriptorAllocator, drawData);
    // texture.UpdateDescriptors();
    // vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->Layout(), 0, 1, &descriptorSet,
    // 0, nullptr);
  }
};

} // namespace Engine::Graphics::Materials
