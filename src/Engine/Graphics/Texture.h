#pragma once

#include "DescriptorHandling.h"
#include "Image.h"
#include "Maths/Dimension.h"
#include "VulkanUtil.h"

namespace Engine::Graphics {
template <uint8_t D> class Texture : public AllocatedImage<D> {
  VkSampler sampler;
  friend class GPUObjectManager;

public:
  inline Texture() : AllocatedImage<D>(), sampler(VK_NULL_HANDLE) {}
  inline Texture(AllocatedImage<D> const &allocatedImage, VkSampler sampler)
      : AllocatedImage<D>(allocatedImage), sampler(sampler) {};

  inline VkDescriptorImageInfo BindInDescriptor(VkImageLayout layout) const override;

  inline void UpdateDescriptors(DescriptorWriter &writer, VkDescriptorSet const &descriptorSet, uint8_t binding = 0,
                                VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) const;

  VkDescriptorSet AddToImGui() const;
};

using Texture1D = Texture<1>;
using Texture2D = Texture<2>;
using Texture3D = Texture<3>;

template <uint8_t D> inline VkDescriptorImageInfo Texture<D>::BindInDescriptor(VkImageLayout layout) const {
  auto res = AllocatedImage<D>::BindInDescriptor(layout);
  res.sampler = sampler;
  return res;
}

template <uint8_t D>
inline void Texture<D>::UpdateDescriptors(DescriptorWriter &writer, VkDescriptorSet const &descriptorSet,
                                          uint8_t binding, VkImageLayout layout) const {
  writer.WriteImage(binding, *this, layout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  writer.UpdateSet(descriptorSet);
  writer.Clear();
}

#include "backends/imgui_impl_vulkan.h"

template <uint8_t D> inline VkDescriptorSet Texture<D>::AddToImGui() const {
  return ImGui_ImplVulkan_AddTexture(sampler, Image<D>::imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

} // namespace Engine::Graphics
