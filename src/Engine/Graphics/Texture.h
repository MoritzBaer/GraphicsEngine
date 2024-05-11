#pragma once

#include "DescriptorHandling.h"
#include "Image.h"
#include "Maths/Dimension.h"
#include "VulkanUtil.h"

namespace Engine::Graphics {
template <uint8_t D> class Texture : public AllocatedImage<D> {
  VkSampler sampler;

public:
  inline Texture(){};
  inline void Create(Maths::Dimension<D> dimension, VkFilter magFilter = VK_FILTER_LINEAR,
                     VkFilter minFilter = VK_FILTER_LINEAR, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
                     bool mipped = true, VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT);
  template <typename T>
  inline void Create(Maths::Dimension<D> dimension, T const *data, VkFilter magFilter = VK_FILTER_LINEAR,
                     VkFilter minFilter = VK_FILTER_LINEAR, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
                     bool mipped = true, VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT);

  inline Texture(Maths::Dimension<D> dimension, VkFilter magFilter = VK_FILTER_LINEAR,
                 VkFilter minFilter = VK_FILTER_LINEAR, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, bool mipped = true,
                 VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT) {
    Create(dimension, magFilter, minFilter, format, mipped, msaaSamples);
  }

  template <typename T>
  inline Texture(Maths::Dimension<D> dimension, T const *data, VkFilter magFilter = VK_FILTER_LINEAR,
                 VkFilter minFilter = VK_FILTER_LINEAR, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, bool mipped = true,
                 VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT) {
    Create(dimension, data, magFilter, minFilter, format, mipped, msaaSamples);
  }

  // I have no idea what happens if the given dimension doesn't match the image's extent
  template <typename T> inline void SetPixels(T const *data, Maths::Dimension<D> dimension);
  template <typename T> inline void SetPixels(T const *data) { SetPixels(data, Image<D>::GetExtent()); }

  inline VkDescriptorImageInfo BindInDescriptor(VkImageLayout layout) const override;

  inline void UpdateDescriptors(VkDescriptorSet const &descriptorSet,
                                VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) const;

  inline virtual void Destroy() const override;
};

using Texture1D = Texture<1>;
using Texture2D = Texture<2>;
using Texture3D = Texture<3>;

template <uint8_t D>
inline void Texture<D>::Create(Maths::Dimension<D> dimension, VkFilter magFilter, VkFilter minFilter, VkFormat format,
                               bool mipped, VkSampleCountFlagBits msaaSamples) {
  AllocatedImage<D>::Create(
      format, vkutil::DimensionToExtent(dimension),
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_IMAGE_ASPECT_COLOR_BIT, (mipped ? static_cast<uint32_t>(std::floor(std::log2(dimension.maxEntry()))) : 0) + 1,
      1, msaaSamples);

  VkSamplerCreateInfo samplerInfo{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .magFilter = magFilter, .minFilter = minFilter};

  InstanceManager::CreateSampler(&samplerInfo, &sampler);
}

template <uint8_t D> inline VkDescriptorImageInfo Texture<D>::BindInDescriptor(VkImageLayout layout) const {
  auto res = AllocatedImage<D>::BindInDescriptor(layout);
  res.sampler = sampler;
  return res;
}

template <uint8_t D>
inline void Texture<D>::UpdateDescriptors(VkDescriptorSet const &descriptorSet, VkImageLayout layout) const {
  DescriptorWriter writer{};
  writer.WriteImage(0, *this, layout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  writer.UpdateSet(descriptorSet);
}

template <uint8_t D> inline void Texture<D>::Destroy() const {
  InstanceManager::DestroySampler(sampler);
  AllocatedImage<D>::Destroy();
}

template <uint8_t D>
template <typename T>
inline void Texture<D>::Create(Maths::Dimension<D> dimension, T const *data, VkFilter magFilter, VkFilter minFilter,
                               VkFormat format, bool mipped, VkSampleCountFlagBits msaaSamples) {

  Create(dimension, magFilter, minFilter, format, mipped, msaaSamples);
  SetPixels(data);
}

template <uint8_t D>
template <typename T>
inline void Texture<D>::SetPixels(T const *data, Maths::Dimension<D> dimension) {

  auto extent = vkutil::DimensionToExtent(dimension);
  Buffer<T> pixelBuffer{data, extent.depth * extent.width * extent.height, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VMA_MEMORY_USAGE_CPU_TO_GPU};

  auto copy = GPUMemoryManager::CopyBufferToImage(pixelBuffer, *this, dimension);
  auto transition = Image<D>::Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  CompositeCommand copyAndTransition{&copy, &transition};
  Renderer::ImmediateSubmit(&copyAndTransition);

  pixelBuffer.Destroy();
}

} // namespace Engine::Graphics
