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
  inline void BindDescriptors(std::vector<VkDescriptorSet> &descriptorSets, DescriptorAllocator &descriptorAllocator,
                              DescriptorWriter &writer, UniformBinding const &uniform) const override {
    descriptorSets.push_back(pipeline->AllocateForLayout(0, descriptorAllocator));
    uniform.WriteToDescriptorSet(writer, descriptorSets.back(), 0);
    descriptorSets.push_back(pipeline->AllocateForLayout(1, descriptorAllocator));
    albedo.UpdateDescriptors(writer, descriptorSets.back(), 0);
    normal.UpdateDescriptors(writer, descriptorSets.back(), 1);
  }
};

} // namespace Engine::Graphics::Materials