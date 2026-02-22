#pragma once

#include "Command.h"
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
  VmaAllocation allocation;
  VkImageView imageView;
  Maths::Dimension<Dimension> imageDimension;
  VkFormat imageFormat;
  VkImageLayout currentLayout;
  VkImageAspectFlags aspect;

  friend class GPUMemoryManager;
  friend class GPUObjectManager;

public:
  inline static const VkImageType IMAGE_TYPE;
  inline static const VkImageViewType VIEW_TYPE;

  inline Image(VkImage image, VmaAllocation allocation, VkImageView imageView, Maths::Dimension<Dimension> imageExtent,
               VkFormat imageFormat, VkImageLayout currentLayout, VkImageAspectFlags aspect)
      : image(image), imageView(imageView), imageDimension(imageExtent), imageFormat(imageFormat),
        currentLayout(currentLayout), aspect(aspect), allocation(allocation) {}
  inline Image()
      : Image(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, Maths::Dimension<Dimension>::Zero, VK_FORMAT_UNDEFINED,
              VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_ASPECT_NONE) {}
  inline Image(VkImage image, VkImageView imageView, Maths::Dimension<Dimension> imageExtent, VkFormat imageFormat,
               VkImageLayout currentLayout, VkImageAspectFlags aspect)
      : Image(image, VK_NULL_HANDLE, imageView, imageExtent, imageFormat, currentLayout, aspect) {}
  inline Image(Image<Dimension> const &other)
      : Image(other.image, other.allocation, other.imageView, other.imageDimension, other.imageFormat,
              other.currentLayout, other.aspect) {}

  inline Command *Transition(VkImageLayout const &newLayout);
  inline vkutil::BlitImageCommand *BlitTo(Image<Dimension> const &target, VkFilter filter = VK_FILTER_LINEAR) const;
  inline VkRenderingAttachmentInfo BindAsColourAttachment(VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                                                          VkClearColorValue const &clearColour = {0, 0, 0, 0}) const;
  inline VkRenderingAttachmentInfo BindAsDepthAttachment(VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                         VkClearDepthStencilValue const &clearValue = {
                                                             .depth = 1.0f}) const;
  inline virtual VkDescriptorImageInfo BindInDescriptor(VkImageLayout layout) const;

  inline Maths::Dimension<Dimension> const & GetExtent() const { return imageDimension; }
};

using Image1 = Image<1>;
using Image2 = Image<2>;
using Image3 = Image<3>;

// IMPLEMENTATIONS

template <> const VkImageType Engine::Graphics::Image<1>::IMAGE_TYPE = VK_IMAGE_TYPE_1D;
template <> const VkImageType Engine::Graphics::Image<2>::IMAGE_TYPE = VK_IMAGE_TYPE_2D;
template <> const VkImageType Engine::Graphics::Image<3>::IMAGE_TYPE = VK_IMAGE_TYPE_3D;

template <> const VkImageViewType Engine::Graphics::Image<1>::VIEW_TYPE = VK_IMAGE_VIEW_TYPE_1D;
template <> const VkImageViewType Engine::Graphics::Image<2>::VIEW_TYPE = VK_IMAGE_VIEW_TYPE_2D;
template <> const VkImageViewType Engine::Graphics::Image<3>::VIEW_TYPE = VK_IMAGE_VIEW_TYPE_3D;

template <uint8_t Dimension> inline Command *Image<Dimension>::Transition(VkImageLayout const &newLayout) {
  if (newLayout == currentLayout) {
    return new vkutil::NoOpCommand(); // TODO: Return NOOP
  }

  auto result =
      new vkutil::PipelineBarrierCommand({vkinit::ImageMemoryBarrier(image, currentLayout, newLayout, aspect)});
  currentLayout = newLayout;
  return result;
}

template <uint8_t Dimension>
inline vkutil::BlitImageCommand *Image<Dimension>::BlitTo(Image<Dimension> const &target, VkFilter filter) const {
  auto imageExtent = vkutil::DimensionToExtent(imageDimension);
  auto targetExtent = vkutil::DimensionToExtent(target.imageDimension);
  VkImageBlit2 blitRegion{
      .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
      .srcSubresource = {.aspectMask = aspect, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
      .srcOffsets =
          {
              {0, 0, 0},
              {static_cast<int32_t>(imageExtent.width), static_cast<int32_t>(imageExtent.height),
               static_cast<int32_t>(imageExtent.depth)},
          },
      .dstSubresource = {.aspectMask = target.aspect, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
      .dstOffsets = {
          {0, 0, 0},
          {static_cast<int32_t>(targetExtent.width), static_cast<int32_t>(targetExtent.height),
           static_cast<int32_t>(targetExtent.depth)},
      }};

  return new vkutil::BlitImageCommand(image, target.image, {blitRegion}, filter);
}

template <uint8_t Dimension>
// Sets rendering extent as image extent. TODO: Think about if this makes sense
inline VkRenderingAttachmentInfo Image<Dimension>::BindAsColourAttachment(VkAttachmentLoadOp loadOp,
                                                                          VkClearColorValue const &clearColour) const {
  return {.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
          .imageView = imageView,
          .imageLayout = currentLayout,
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
          .imageLayout = currentLayout,
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
