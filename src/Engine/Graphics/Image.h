#pragma once

#include "InstanceManager.h"
#include "Maths/Dimension.h"
#include "MemoryAllocator.h"
#include "VulkanUtil.h"
#include "vulkan/vulkan.h"

namespace Engine::Graphics {

// Image is not responsible for image object creation and therefore also doesn't destroy it
template <uint8_t Dimension> class Image {
protected:
  VkImage image;
  VkImageView imageView;
  Maths::Dimension<Dimension> imageDimension;
  VkFormat imageFormat;
  VkImageLayout currentLayout;

  friend class GPUMemoryManager;
  friend class GPUObjectManager;

public:
  static const VkImageType IMAGE_TYPE;
  static const VkImageViewType VIEW_TYPE;

  inline Image()
      : image(VK_NULL_HANDLE), imageView(VK_NULL_HANDLE), imageDimension(Maths::Dimension<Dimension>::Zero()),
        imageFormat(VK_FORMAT_UNDEFINED), currentLayout(VK_IMAGE_LAYOUT_UNDEFINED) {}
  inline Image(VkImage image, VkImageView imageView, Maths::Dimension<Dimension> imageExtent, VkFormat imageFormat,
               VkImageLayout currentLayout)
      : image(image), imageView(imageView), imageDimension(imageExtent), imageFormat(imageFormat),
        currentLayout(currentLayout) {}
  inline Image(Image<Dimension> const &other)
      : Image(other.image, other.imageView, other.imageDimension, other.imageFormat, other.currentLayout) {}

  inline vkutil::PipelineBarrierCommand Transition(VkImageLayout const &newLayout);
  inline vkutil::BlitImageCommand BlitTo(Image<Dimension> const &target) const;
  inline VkRenderingAttachmentInfo BindAsColourAttachment(VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                                                          VkClearColorValue const &clearColour = {0, 0, 0, 0}) const;
  inline VkRenderingAttachmentInfo BindAsDepthAttachment(VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                         VkClearDepthStencilValue const &clearValue = {
                                                             .depth = 1.0f}) const;
  inline virtual VkDescriptorImageInfo BindInDescriptor(VkImageLayout layout) const;

  inline Maths::Dimension<Dimension> GetExtent() const { return imageDimension; }
};

template <uint8_t Dimension> class AllocatedImage : public Image<Dimension> {
private:
  VmaAllocation allocation;

  friend class GPUObjectManager;

public:
  inline AllocatedImage() : Image<Dimension>(), allocation(VK_NULL_HANDLE) {}
  inline AllocatedImage(Image<Dimension> const &image, VmaAllocation allocation)
      : Image<Dimension>(image), allocation(allocation) {}
  inline AllocatedImage(AllocatedImage<Dimension> const &other) : AllocatedImage(other, other.allocation) {}
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
inline vkutil::PipelineBarrierCommand Image<Dimension>::Transition(VkImageLayout const &newLayout) {
  auto result = vkutil::PipelineBarrierCommand({vkinit::ImageMemoryBarrier(image, currentLayout, newLayout)});
  currentLayout = newLayout;
  return result;
}

template <uint8_t Dimension>
inline vkutil::BlitImageCommand Image<Dimension>::BlitTo(Image<Dimension> const &target) const {
  auto imageExtent = vkutil::DimensionToExtent(imageDimension);
  auto targetExtent = vkutil::DimensionToExtent(target.imageDimension);
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
          {static_cast<int32_t>(targetExtent.width), static_cast<int32_t>(targetExtent.height),
           static_cast<int32_t>(targetExtent.depth)},
      }};

  return vkutil::BlitImageCommand(image, target.image, {blitRegion});
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
  return {
      .imageView = imageView,
      .imageLayout = layout,
  };
}

} // namespace Engine::Graphics
