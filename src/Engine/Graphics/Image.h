#pragma once

#include "CommandQueue.h"
#include "InstanceManager.h"
#include "Maths/Dimension.h"
#include "MemoryAllocator.h"
#include "VulkanUtil.h"
#include "vulkan/vulkan.h"

namespace Engine::Graphics {

// Image is not responsible for image object creation and therefore also doesn't destroy it
template <uint8_t Dimension> class Image : public Destroyable {
protected:
  VkImage image;
  VkImageView imageView;
  VkExtent3D imageExtent;
  VkFormat imageFormat;
  VkImageLayout currentLayout;

  static const VkImageType IMAGE_TYPE;
  static const VkImageViewType VIEW_TYPE;

public:
  inline void Create(VkImage const &image, VkExtent3D const &extent, VkFormat const &format,
                     VkImageAspectFlags aspectMask, uint32_t mipLevels, uint32_t arrayLayers,
                     VkSampleCountFlagBits msaaSamples);

  inline PipelineBarrierCommand Transition(VkImageLayout const &newLayout);
  inline BlitImageCommand BlitTo(Image<Dimension> const &target) const;
  inline VkRenderingAttachmentInfo BindAsColourAttachment(VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                                                          VkClearColorValue const &clearColour = {0, 0, 0, 0}) const;
  inline VkRenderingAttachmentInfo BindAsDepthAttachment(VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                         VkClearDepthStencilValue const &clearValue = {
                                                             .depth = 1.0f}) const;
  inline VkDescriptorImageInfo BindInDescriptor(VkImageLayout layout) const;

  inline Maths::Dimension<Dimension> GetExtent() const;

  void Destroy() const;
};

template <uint8_t Dimension> class AllocatedImage : public Image<Dimension> {
private:
  VmaAllocation allocation;

  // friend class Renderer;

public:
  inline AllocatedImage() = default;
  inline void Create(VkFormat format, VkExtent3D extent, VkImageUsageFlags usage, VkImageAspectFlags aspectMask,
                     uint32_t mipLevels = 1, uint32_t arrayLayers = 1,
                     VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT);
  inline void Destroy() const;
};

using Image1 = Image<1>;
using Image2 = Image<2>;
using Image3 = Image<3>;

using AllocatedImage1 = AllocatedImage<1>;
using AllocatedImage2 = AllocatedImage<2>;
using AllocatedImage3 = AllocatedImage<3>;

// IMPLEMENTATIONS

template <> const VkImageType Engine::Graphics::Image<1>::IMAGE_TYPE = VK_IMAGE_TYPE_1D;
template <> const VkImageType Engine::Graphics::Image<2>::IMAGE_TYPE = VK_IMAGE_TYPE_2D;
template <> const VkImageType Engine::Graphics::Image<3>::IMAGE_TYPE = VK_IMAGE_TYPE_3D;

template <> const VkImageViewType Engine::Graphics::Image<1>::VIEW_TYPE = VK_IMAGE_VIEW_TYPE_1D;
template <> const VkImageViewType Engine::Graphics::Image<2>::VIEW_TYPE = VK_IMAGE_VIEW_TYPE_2D;
template <> const VkImageViewType Engine::Graphics::Image<3>::VIEW_TYPE = VK_IMAGE_VIEW_TYPE_3D;

template <uint8_t Dimension>
inline void Engine::Graphics::AllocatedImage<Dimension>::Create(VkFormat format, VkExtent3D extent,
                                                                VkImageUsageFlags usage, VkImageAspectFlags aspectMask,
                                                                uint32_t mipLevels, uint32_t arrayLayers,
                                                                VkSampleCountFlagBits msaaSamples) {
  VkImageCreateInfo imageCreateInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = Image<Dimension>::IMAGE_TYPE,
      .format = format,
      .extent = extent,
      .mipLevels = mipLevels,
      .arrayLayers = arrayLayers,
      .samples = msaaSamples,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = usage,
  };

  VkImage im;
  mainAllocator.CreateImage(&imageCreateInfo, &im, &allocation);
  Image<Dimension>::Create(im, extent, format, aspectMask, mipLevels, arrayLayers, msaaSamples);
}

template <uint8_t Dimension>
inline void Image<Dimension>::Create(VkImage const &image, VkExtent3D const &extent, VkFormat const &format,
                                     VkImageAspectFlags aspectMask, uint32_t mipLevels, uint32_t arrayLayers,
                                     VkSampleCountFlagBits msaaSamples) {
  this->image = image;
  imageExtent = extent;
  imageFormat = format;
  currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  VkImageViewCreateInfo imageViewCreateInfo{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                            .image = image,
                                            .viewType = VIEW_TYPE,
                                            .format = imageFormat,
                                            .subresourceRange = {.aspectMask = aspectMask,
                                                                 .baseMipLevel = 0,
                                                                 .levelCount = mipLevels,
                                                                 .baseArrayLayer = 0,
                                                                 .layerCount = arrayLayers}};

  InstanceManager::CreateImageView(&imageViewCreateInfo, &imageView);
}

template <uint8_t Dimension>
inline PipelineBarrierCommand Image<Dimension>::Transition(VkImageLayout const &newLayout) {
  auto result = PipelineBarrierCommand({vkinit::ImageMemoryBarrier(image, currentLayout, newLayout)});
  currentLayout = newLayout;
  return result;
}

template <uint8_t Dimension> inline BlitImageCommand Image<Dimension>::BlitTo(Image<Dimension> const &target) const {
  VkImageBlit2 blitRegion{
      .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
      .srcSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
      .srcOffsets =
          {
              {0, 0, 0},
              {static_cast<int32_t>(imageExtent.width), static_cast<int32_t>(imageExtent.height),
               static_cast<int32_t>(imageExtent.depth)},
          },
      .dstSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
      .dstOffsets = {
          {0, 0, 0},
          {static_cast<int32_t>(target.imageExtent.width), static_cast<int32_t>(target.imageExtent.height),
           static_cast<int32_t>(target.imageExtent.depth)},
      }};

  return BlitImageCommand(image, target.image, {blitRegion});
}

template <uint8_t Dimension>
// Sets rendering extent as image extent. TODO: Think about if this makes sense
inline VkRenderingAttachmentInfo Image<Dimension>::BindAsColourAttachment(VkAttachmentLoadOp loadOp,
                                                                          VkClearColorValue const &clearColour) const {
  return {.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
          .imageView = imageView,
          .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
          .loadOp = loadOp,
          .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
          .clearValue{.color = clearColour}};
}

template <uint8_t Dimension>
// Sets rendering extent as image extent. TODO: Think about if this makes sense
inline VkRenderingAttachmentInfo
Image<Dimension>::BindAsDepthAttachment(VkAttachmentLoadOp loadOp, VkClearDepthStencilValue const &clearValue) const {
  return {.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
          .imageView = imageView,
          .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
          .loadOp = loadOp,
          .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
          .clearValue{.depthStencil = clearValue}};
}

template <uint8_t Dimension>
inline VkDescriptorImageInfo Image<Dimension>::BindInDescriptor(VkImageLayout layout) const {
  return {.imageView = imageView, .imageLayout = layout};
}

template <> inline Maths::Dimension<1> Image<1>::GetExtent() const { return Maths::Dimension<1>(imageExtent.width); }
template <> inline Maths::Dimension<2> Image<2>::GetExtent() const {
  return Maths::Dimension<2>(imageExtent.width, imageExtent.height);
}
template <> inline Maths::Dimension<3> Image<3>::GetExtent() const {
  return Maths::Dimension<3>(imageExtent.width, imageExtent.height, imageExtent.depth);
}

template <uint8_t Dimension> inline void Image<Dimension>::Destroy() const {
  InstanceManager::DestroyImageView(imageView);
}

template <uint8_t Dimension> inline void AllocatedImage<Dimension>::Destroy() const {
  Image<Dimension>::Destroy();
  mainAllocator.DestroyImage(Image<Dimension>::image, allocation);
}

} // namespace Engine::Graphics
