#pragma once

#include "Material.h"
#include "Maths/Dimension.h"
#include "Texture.h"

namespace Engine::Graphics {

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
  inline void AppendData(UniformAggregate &aggregate) const { aggregate.PushData(&colour); }
  inline void Bind(VkCommandBuffer const &commandBuffer, VkDescriptorSet const &descriptorSet) const override {
    Material::Bind(commandBuffer, descriptorSet);
    // Create new texture to test if anything degrades during copy

    // Load error texture
    std::vector<uint32_t> errorTextureData(16 * 16, 0xFF00FFFF);
    for (int x = 0; x < 16; x++) {
      for (int y = x % 2; y < 16; y += 2) {
        errorTextureData[x * 16 + y] = 0x000000FF;
      }
    }
    texture.UpdateDescriptors(descriptorSet);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->Layout(), 0, 1, &descriptorSet, 0,
                            nullptr);
  }
};

} // namespace Engine::Graphics
