#pragma once

#include "Graphics/Material.h"
#include "Graphics/Texture.h"

namespace Engine::Graphics::Materials {

struct AlbedoAndBump : public Material {
  Texture2D albedo;
  Texture2D normal;

  AlbedoAndBump(Material const *other) : Material(other) {
    if (AlbedoAndBump const *phong = dynamic_cast<AlbedoAndBump const *>(other)) {
      albedo = phong->albedo;
      normal = phong->normal;
    } else {
      uint32_t white = 0xFFFFFFFF;
      uint32_t normalUp = 0xFFFF0000;
      albedo = Texture2D(Maths::Dimension2(1, 1), &white);
      normal = Texture2D(Maths::Dimension2(1, 1), &normalUp);
    }
  }

  AlbedoAndBump(Pipeline const *pipeline, Texture2D albedo, Texture2D normal)
      : Material(pipeline), albedo(albedo), normal(normal) {}
  AlbedoAndBump(Pipeline const *pipeline, Texture2D albedo) : Material(pipeline), albedo(albedo) {
    uint32_t normalUp = 0xFFFF0000;
    normal = Texture2D(Maths::Dimension2(1, 1), &normalUp);
  }
  AlbedoAndBump(Pipeline const *pipeline) : Material(pipeline) {
    uint32_t white = 0xFFFFFFFF;
    uint32_t normalUp = 0xFFFF0000;
    albedo = Texture2D(Maths::Dimension2(1, 1), &white);
    normal = Texture2D(Maths::Dimension2(1, 1), &normalUp);
  }
  AlbedoAndBump(Pipeline const *pipeline, Maths::Vector3 const &colour) : Material(pipeline) {
    // TODO: Fix overspills
    uint32_t colourData = 0xFF000000 | (uint32_t(colour.x() * 255) << 16) | (uint32_t(colour.y() * 255) << 8) |
                          uint32_t(colour.z() * 255);
    albedo = Texture2D(Maths::Dimension2(1, 1), &colourData);
    uint32_t normalUp = 0xFFFF0000;
    normal = Texture2D(Maths::Dimension2(1, 1), &normalUp);
  }

  inline void AppendData(PushConstantsAggregate &aggregate) const override {}
  inline void Bind(VkCommandBuffer const &commandBuffer, DescriptorAllocator &descriptorAllocator,
                   Buffer<DrawData> const &drawDataBuffer) const override {
    Material::Bind(commandBuffer, descriptorAllocator, drawDataBuffer);
    std::vector<VkDescriptorSet> descriptorSets{};
    descriptorSets.push_back(descriptorAllocator.Allocate(pipeline->DescriptorLayout(0)));
    drawDataBuffer.UpdateDescriptor(descriptorSets.back(), 0);
    descriptorSets.push_back(descriptorAllocator.Allocate(pipeline->DescriptorLayout(1)));
    albedo.UpdateDescriptors(descriptorSets.back(), 0);
    normal.UpdateDescriptors(descriptorSets.back(), 1);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->Layout(), 0,
                            static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
  }
};

} // namespace Engine::Graphics::Materials
