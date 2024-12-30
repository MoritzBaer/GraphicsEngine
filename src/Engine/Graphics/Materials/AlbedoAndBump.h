#pragma once

#include "AssetManager.h"
#include "Game.h"
#include "Graphics/Material.h"
#include "Graphics/Texture.h"

namespace Engine::Graphics::Materials {

struct AlbedoAndBump : public Material {
  Texture2D albedo;
  Texture2D normal;
  Maths::Vector3 hue;
  float specularStrength; // TODO: Extract into Phong
  float phongExponent;

  AlbedoAndBump(Material const *other) : Material(other) {
    if (AlbedoAndBump const *aab = dynamic_cast<AlbedoAndBump const *>(other)) {
      albedo = aab->albedo;
      normal = aab->normal;
      specularStrength = aab->specularStrength;
      phongExponent = aab->phongExponent;
      hue = aab->hue;
    } else {
      ENGINE_ERROR("Tried to initialize AlbedoAndBump from Material of different type!");
    }
  }

  AlbedoAndBump(Pipeline const *pipeline, Texture2D albedo, Texture2D normal, float specularStrength = 0.5f,
                float phongExponent = 16.0f, Maths::Vector3 hue = Maths::Vector3(1.0f, 1.0f, 1.0f))
      : Material(pipeline), albedo(albedo), normal(normal), specularStrength(specularStrength),
        phongExponent(phongExponent), hue(hue) {}

  inline void AppendData(PushConstantsAggregate &aggregate) const override {}
  inline void Bind(VkCommandBuffer const &commandBuffer, DescriptorAllocator &descriptorAllocator,
                   DescriptorWriter &writer, Buffer<DrawData> const &drawDataBuffer) const override {
    Material::Bind(commandBuffer, descriptorAllocator, writer, drawDataBuffer);
    std::vector<VkDescriptorSet> descriptorSets{};
    descriptorSets.push_back(descriptorAllocator.Allocate(pipeline->DescriptorLayout(0)));
    drawDataBuffer.UpdateDescriptor(writer, descriptorSets.back(), 0);
    descriptorSets.push_back(descriptorAllocator.Allocate(pipeline->DescriptorLayout(1)));
    albedo.UpdateDescriptors(writer, descriptorSets.back(), 0);
    normal.UpdateDescriptors(writer, descriptorSets.back(), 1);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->Layout(), 0,
                            static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
  }
};

} // namespace Engine::Graphics::Materials