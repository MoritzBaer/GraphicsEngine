#pragma once

#include "AssetManager.h"
#include "Buffer.h"
#include "Debug/Logging.h"
#include "Graphics/AllocatedMesh.h"
#include "Graphics/Image.h"
#include "Graphics/Material.h"
#include "Graphics/UniformAggregate.h"
#include "Graphics/VulkanUtil.h"
#include "Maths/Dimension.h"
#include "vulkan/vulkan.h"
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <span>
#include <tuple>

namespace Engine::Graphics {

class GPUObjectManager;

template <typename T_GPU> struct VertexBufferBinding {
  VertexBufferT<T_GPU> buffer;
  VkDeviceSize offset;
};

class CommandRecorder;
class RenderPassRecorder;
class MaterialBinder;
class DrawCallRecorder;

template <typename CS_T>
concept CommandSet = requires(CS_T const &cs, CommandRecorder const &cr) {
  { cs(cr) };
};

template <typename RP_T>
concept RenderPassDefinition = requires(RP_T const &rp, RenderPassRecorder const &rpr) {
  { rp(rpr) };
};

template <typename MB_T>
concept MaterialBind = requires(MB_T const &mb, MaterialBinder const &mbr) {
  { mb(mbr) };
};

template <typename DC_T>
concept DrawCall = requires(DC_T const &dc, DrawCallRecorder const &dcr) {
  { dc(dcr) };
};

class CommandRecorder {

  struct RenderPassTarget {
    std::vector<Image2 > drawImages;
    Image2 const *depthImage;
    VkAttachmentLoadOp depthBufferLoadOp;
    Maths::Dimension2 const *extent;
    Maths::Dimension2 const *offset;
  };

  struct RenderPassBuilder {
  private:
    RenderPassTarget underConstruction;
    CommandRecorder const &parentRecorder;

  public:
    RenderPassBuilder(const CommandRecorder &parentRecorder) : underConstruction(), parentRecorder(parentRecorder) {}

    RenderPassBuilder &WithDrawImage(std::same_as<Image2> auto const &...images) {
      if constexpr (sizeof...(images) == 1) {
        // Automatically deduce extent only in this case
        if (!underConstruction.extent) {
          underConstruction.extent = &std::get<0>(std::tie(images...)).GetExtent();
        }
      }
      underConstruction.drawImages = {images...};
      return *this;
    }
    RenderPassBuilder &WithDepthImage(Image2 const &image) {
      underConstruction.depthImage = &image;
      return *this;
    }
    RenderPassBuilder &WithDepthBufferLoadOp(VkAttachmentLoadOp op) {
      underConstruction.depthBufferLoadOp = op;
      return *this;
    }
    RenderPassBuilder &WithExtent(Maths::Dimension2 const &extent) {
      underConstruction.extent = &extent;
      return *this;
    }
    RenderPassBuilder &WithOffset(Maths::Dimension2 const &offset) {
      underConstruction.offset = &offset;
      return *this;
    }
    void As(RenderPassDefinition auto const &definition);
  };

protected:
  VkCommandBuffer const &buffer;

public:
  CommandRecorder(VkCommandBuffer const &buffer) : buffer(buffer) {}

  // +-------- Copy operations --------+

  // Buffer-to-buffer
  template <typename T1, typename T2>
  void RecordCopy(Buffer<T1> const &src, Buffer<T2> const &dst, size_t numBytes, size_t srcOffset,
                  size_t dstOffset) const;
  template <typename T1, typename T2>
  inline void RecordCopy(Buffer<T1> const &src, Buffer<T2> const &dst, size_t srcOffset = 0,
                         size_t dstOffset = 0) const {
    RecordCopy(src, dst, src.PhysicalSize(), srcOffset, dstOffset);
  }

  // Buffer-to-Image
  template <typename T, uint8_t D>
  void RecordCopy(Buffer<T> const &src, Image<D> &dst, Maths::Dimension<D> const &pixels, size_t srcOffset = 0,
                  Maths::Dimension<D> const &dstOffset = Maths::Dimension<D>::Zero) const;
  template <typename T, uint8_t D>
  inline void RecordCopy(Buffer<T> const &src, Image<D> &dst, size_t srcOffset = 0,
                         Maths::Dimension<D> const &dstOffset = Maths::Dimension<D>::Zero) const {
    RecordCopy(src, dst, dst.imageDimension, srcOffset, dstOffset);
  }

  // Image-to-Buffer
  template <typename T, uint8_t D>
  void RecordCopy(Image<D> &src, Buffer<T> const &dst, Maths::Dimension<D> const &pixels,
                  Maths::Dimension<D> const &srcOffset = Maths::Dimension<D>::Zero, size_t dstOffset = 0) const;
  template <typename T, uint8_t D>
  inline void RecordCopy(Image<D> &src, Buffer<T> const &dst,
                         Maths::Dimension<D> const &srcOffset = Maths::Dimension<D>::Zero, size_t dstOffset = 0) const {
    RecordCopy(src, dst, src.imageDimension, srcOffset, dstOffset);
  }

  // +-------- Image operations -------+
  void RecordPipelineBarrier(std::initializer_list<VkImageMemoryBarrier2> const &imageBarriers) const;
  inline void RecordPipelineBarrier(std::same_as<VkImageMemoryBarrier2> auto const &...imageBarriers) const {
    RecordPipelineBarrier(std::initializer_list<VkImageMemoryBarrier2>{imageBarriers...});
  }

  template <uint8_t D>
  void RecordColorImageClear(Image<D> const &image, VkClearColorValue const &clearColour,
                             std::initializer_list<VkImageSubresourceRange> const &subresourceRanges) const;
  template <uint8_t D>
  inline void RecordColorImageClear(Image<D> const &image, VkClearColorValue const &clearColour,
                                    std::same_as<VkImageSubresourceRange> auto const &...subresourceRanges) const {
    RecordColorImageClear({subresourceRanges...});
  }

  template <uint8_t D>
  void RecordBlit(Image<D> const &source, Image<D> const &destination, VkFilter filter,
                  std::initializer_list<VkImageBlit2> const &blitRegions) const;
  template <uint8_t D>
  inline void RecordBlit(Image<D> const &source, Image<D> const &destination, VkFilter filter,
                         std::same_as<VkImageBlit2> auto const &...blitRegions) const {
    RecordBlit(source, destination, filter, {blitRegions...});
  }
  template <uint8_t D> void RecordBlit(Image<D> const &source, Image<D> const &destination, VkFilter filter) const;

  template <uint8_t D> void RecordTransition(Image<D> &image, VkImageLayout newLayout) const;

  void RecordWithBoundPipeline(Graphics::Pipeline const &pipeline, VkPipelineBindPoint bindPoint,
                               MaterialBind auto const &binder) const;

  // +---------- Render pass ----------+
  void RecordRenderPass(std::span<Image2> const &drawImages, Image2 const &depthImage,
                        VkAttachmentLoadOp depthBufferLoadOp, Maths::Dimension2 const &extent,
                        Maths::Dimension2 const &offset, RenderPassDefinition auto const &recorder) const;
  inline RenderPassBuilder RecordRenderPass() const { return RenderPassBuilder(*this); }

  void RecordViewports(std::initializer_list<VkViewport> const &viewports) const;
  inline void RecordViewports(std::same_as<VkViewport> auto const &...viewports) const {
    RecordViewports({viewports...});
  }

  void RecordScissors(std::initializer_list<VkRect2D> const &scissors) const;
  inline void RecordScissors(std::same_as<VkRect2D> auto const &...scissors) const { RecordScissors({scissors...}); }
};

class MaterialBinder : protected CommandRecorder {

  VkPipelineLayout boundLayout;
  VkPipelineBindPoint usedBindpoint;

public:
  MaterialBinder(VkCommandBuffer const & buffer, VkPipelineLayout layout, VkPipelineBindPoint bindPoint)
      : CommandRecorder(buffer), boundLayout(layout), usedBindpoint(bindPoint) {}

  using CommandRecorder::RecordBlit;
  using CommandRecorder::RecordColorImageClear;
  using CommandRecorder::RecordCopy;
  using CommandRecorder::RecordPipelineBarrier;
  using CommandRecorder::RecordTransition;

  // +-------- Material binding -------+
  void RecordDescriptorBind(std::span<VkDescriptorSet const> const &descriptors) const;
  inline void RecordDescriptorBind(std::same_as<VkDescriptorSet> auto... descriptors) const {
    RecordDescriptorBind(std::span(std::initializer_list{descriptors...}));
  }
  template <typename T> void RecordPushConstantSet(T const &constants, VkShaderStageFlags stage) const;

  // +----------- Dispatch ------------+
  inline void RecordDispatch(uint32_t workerGroupsX, uint32_t workerGroupsY = 1, uint32_t workerGroupsZ = 1) const {
    vkCmdDispatch(buffer, workerGroupsX, workerGroupsY, workerGroupsZ);
  }
};

class RenderPassRecorder : protected CommandRecorder {
public:
  RenderPassRecorder(VkCommandBuffer const &buffer) : CommandRecorder(buffer) {}

  using CommandRecorder::RecordBlit;
  using CommandRecorder::RecordColorImageClear;
  using CommandRecorder::RecordCopy;
  using CommandRecorder::RecordPipelineBarrier;
  using CommandRecorder::RecordScissors;
  using CommandRecorder::RecordTransition;
  using CommandRecorder::RecordViewports;

  void RecordWithBoundPipeline(Graphics::Pipeline const &pipeline, VkPipelineBindPoint bindPoint,
                               DrawCall auto const &drawCall) const;
};

class DrawCallRecorder : protected RenderPassRecorder, protected MaterialBinder {

public:
  DrawCallRecorder(VkCommandBuffer const & buffer, VkPipelineLayout layout, VkPipelineBindPoint bindPoint)
      : RenderPassRecorder(buffer), MaterialBinder(buffer, layout, bindPoint) {}

  using RenderPassRecorder::RecordScissors;
  using RenderPassRecorder::RecordViewports;
  using RenderPassRecorder::RecordBlit;
  using RenderPassRecorder::RecordColorImageClear;
  using RenderPassRecorder::RecordCopy;
  using RenderPassRecorder::RecordPipelineBarrier;
  using RenderPassRecorder::RecordTransition;
  using MaterialBinder::RecordPushConstantSet;
  using MaterialBinder::RecordDescriptorBind;
  using MaterialBinder::RecordDispatch;

  template <std::integral T>
  void RecordIndexedDraw(Buffer<T> const &indexBuffer, uint32_t indexCount, uint32_t instanceCount = 1,
                         uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) const;
  template <std::integral T>
  inline void RecordIndexedDraw(Buffer<T> const &indexBuffer, uint32_t instanceCount = 1, uint32_t firstIndex = 0,
                                int32_t vertexOffset = 0, uint32_t firstInstance = 0) const {
    RecordIndexedDraw(indexBuffer, indexBuffer.Size(), instanceCount, firstIndex, vertexOffset, firstInstance);
  }

  inline void RecordMeshDraw(AllocatedMesh const &mesh) const { RecordIndexedDraw(mesh.indexBuffer); }

  template <typename T_GPU> void RecordVertexBufferBind(std::span<VertexBufferBinding<T_GPU>> const &bindings) const;
  template <typename T_GPU>
  void RecordVertexBufferBind(std::initializer_list<VertexBufferBinding<T_GPU>> const &bindings) const {
    RecordVertexBufferBind(std::span<VertexBufferBinding<T_GPU>>(bindings));
  }
  template <typename T_GPU>
  inline void RecordVertexBufferBind(std::initializer_list<VertexBufferT<T_GPU>> const &buffers) const {
    std::vector<VertexBufferBinding<T_GPU>> bindings{};
    bindings.resize(buffers.size());
    std::transform(buffers.begin(), buffers.end(), bindings.begin(), [](VertexBufferT<T_GPU> const &buf) {
      return VertexBufferBinding<T_GPU>{.buffer = buf, .offset = 0};
    });
    RecordVertexBufferBind(std::span(bindings));
  }
  template <typename T_GPU>
  void RecordVertexBufferBind(std::same_as<VertexBufferBinding<T_GPU>> auto const &...bindings) const {
    RecordVertexBufferBind({bindings...});
  }
  template <typename T_GPU>
  void RecordVertexBufferBind(std::same_as<VertexBufferT<T_GPU>> auto const &...buffers) const {
    RecordVertexBufferBind({buffers...});
  }

  template <std::integral T> void RecordIndexBufferBind(Buffer<T> const &indexBuffer, VkDeviceSize offset = 0) const;
  inline void RecordDraw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0,
                         uint32_t firstInstance = 0) const {
    vkCmdDraw(RenderPassRecorder::buffer, vertexCount, instanceCount, firstVertex, firstInstance);
  }
  template <typename T>
  inline void RecordDraw(Buffer<T> const &vertexBuffer, uint32_t instanceCount = 1, uint32_t firstInstance = 0) const {
    RecordVertexBufferBind<T>(VertexBufferT<T>(vertexBuffer));
    RecordDraw(vertexBuffer.Size(), instanceCount, 0, firstInstance);
  }
};

class CommandQueue {
  friend class GPUObjectManager;

private:
  VkCommandBuffer mainBuffer;
  VkCommandPool pool;

public:
  CommandQueue(VkCommandBuffer mainBuffer, VkCommandPool pool) : mainBuffer(mainBuffer), pool(pool) {}
  CommandQueue() : CommandQueue(VK_NULL_HANDLE, VK_NULL_HANDLE) {}

  CommandRecorder GetRecorder(VkCommandBufferUsageFlags flags = 0) const;
  VkCommandBufferSubmitInfo EnqueueCommandRecord(CommandRecorder const &recorder) const;

  inline VkCommandBufferSubmitInfo EnqueueCommands(CommandSet auto const &commands,
                                                   VkCommandBufferUsageFlags flags = 0) const {
    auto const rec = GetRecorder(flags);
    commands(rec);
    return EnqueueCommandRecord(rec);
  }
};

template <typename T1, typename T2>
inline void CommandRecorder::RecordCopy(Buffer<T1> const &src, Buffer<T2> const &dst, size_t numBytes, size_t srcOffset,
                                        size_t dstOffset) const {
  VkBufferCopy region{.srcOffset = srcOffset, .dstOffset = dstOffset, .size = numBytes};
  vkCmdCopyBuffer(buffer, src.buffer, dst.buffer, 1, &region);
}

template <typename T, uint8_t D>
inline void CommandRecorder::RecordCopy(Buffer<T> const &src, Image<D> &dst, Maths::Dimension<D> const &pixels,
                                        size_t srcOffset, Maths::Dimension<D> const &dstOffset) const {
  RecordTransition(dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  VkBufferImageCopy region{.bufferOffset = srcOffset,
                           .imageSubresource = {.aspectMask = dst.aspect, .layerCount = 1},
                           .imageOffset = vkutil::DimensionToOffset(dstOffset),
                           .imageExtent = vkutil::DimensionToExtent(pixels)};

  vkCmdCopyBufferToImage(buffer, src.buffer, dst.image, dst.currentLayout, 1, &region);
}

template <typename T, uint8_t D>
inline void CommandRecorder::RecordCopy(Image<D> &src, Buffer<T> const &dst, Maths::Dimension<D> const &pixels,
                                        Maths::Dimension<D> const &srcOffset, size_t dstOffset) const {
  RecordTransition(src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  VkBufferImageCopy region{.bufferOffset = dstOffset,
                           .imageSubresource = {.aspectMask = src.aspect, .layerCount = 1},
                           .imageOffset = vkutil::DimensionToOffset(srcOffset),
                           .imageExtent = vkutil::DimensionToExtent(pixels)};
  vkCmdCopyImageToBuffer(buffer, src.image, src.imageDimension, dst.buffer, 1, &region);
}

inline void CommandRecorder::RecordPipelineBarrier(std::initializer_list<VkImageMemoryBarrier2> const &barriers) const {
  VkDependencyInfo const dependency = {.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                                       .imageMemoryBarrierCount = static_cast<uint32_t>(barriers.size()),
                                       .pImageMemoryBarriers = barriers.begin()};
  vkCmdPipelineBarrier2(buffer, &dependency);
}

template <uint8_t D>
inline void
CommandRecorder::RecordColorImageClear(Image<D> const &image, VkClearColorValue const &clearColour,
                                       std::initializer_list<VkImageSubresourceRange> const &subresourceRanges) const {
  vkCmdClearColorImage(buffer, image.image, image.currentLayout, clearColour, subresourceRanges.size(),
                       subresourceRanges.begin());
}

template <uint8_t D>
inline void CommandRecorder::RecordBlit(Image<D> const &source, Image<D> const &destination, VkFilter filter) const {
  auto const srcExtent = vkutil::DimensionToExtent(source.imageDimension);
  auto const dstExtent = vkutil::DimensionToExtent(destination.imageDimension);
  VkImageBlit2 blitRegion{
      .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
      .srcSubresource = {.aspectMask = source.aspect, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
      .srcOffsets = {{0, 0, 0},
                     {static_cast<int32_t>(srcExtent.width), static_cast<int32_t>(srcExtent.height),
                      static_cast<int32_t>(srcExtent.depth)}},

      .dstSubresource = {.aspectMask = source.aspect, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
      .dstOffsets = {{0, 0, 0},
                     {static_cast<int32_t>(dstExtent.width), static_cast<int32_t>(dstExtent.height),
                      static_cast<int32_t>(dstExtent.depth)}},

  };
  RecordBlit(source, destination, filter, blitRegion);
}

template <uint8_t D>
inline void CommandRecorder::RecordBlit(Image<D> const &source, Image<D> const &destination, VkFilter filter,
                                        std::initializer_list<VkImageBlit2> const &blitRegions) const {
  VkBlitImageInfo2 const blitInfo = {.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
                                     .srcImage = source.image,
                                     .srcImageLayout = source.currentLayout,
                                     .dstImage = destination.image,
                                     .dstImageLayout = destination.currentLayout,
                                     .regionCount = static_cast<uint32_t>(blitRegions.size()),
                                     .pRegions = blitRegions.begin(),
                                     .filter = filter};
  vkCmdBlitImage2(buffer, &blitInfo);
}

template <uint8_t D> inline void CommandRecorder::RecordTransition(Image<D> &image, VkImageLayout newLayout) const {
  if (image.currentLayout == newLayout) {
    return;
  }

  RecordPipelineBarrier(vkinit::ImageMemoryBarrier(image.image, image.currentLayout, newLayout, image.aspect));
  image.currentLayout = newLayout;
}

inline void Engine::Graphics::CommandRecorder::RecordRenderPass(std::span<Image2> const &drawImages,
                                                                Image2 const &depthImage,
                                                                VkAttachmentLoadOp depthBufferLoadOp,
                                                                Maths::Dimension2 const &extent,
                                                                Maths::Dimension2 const &offset,
                                                                RenderPassDefinition auto const &definition) const {
  std::vector<VkRenderingAttachmentInfo> colourAttachmentInfos{};
  colourAttachmentInfos.resize(drawImages.size());
  std::transform(drawImages.begin(), drawImages.end(), colourAttachmentInfos.begin(),
                 [](Image2 const &image) { return image.BindAsColourAttachment(); });
  auto const depthAttachment = depthImage.BindAsDepthAttachment(depthBufferLoadOp);
  VkRenderingInfo const renderingInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = VkRect2D{.offset = {.x = static_cast<int32_t>(offset.x()), .y = static_cast<int32_t>(offset.y())},
                             .extent = {.width = extent.x(), .height = extent.y()}},
      .layerCount = 1,
      .colorAttachmentCount = static_cast<uint32_t>(colourAttachmentInfos.size()),
      .pColorAttachments = colourAttachmentInfos.data(),
      .pDepthAttachment = &depthAttachment};

  vkCmdBeginRendering(buffer, &renderingInfo);

  definition(RenderPassRecorder{buffer});

  vkCmdEndRendering(buffer);
}

inline void CommandRecorder::RecordViewports(std::initializer_list<VkViewport> const &viewports) const {
  vkCmdSetViewport(buffer, 0, viewports.size(), viewports.begin());
}

inline void CommandRecorder::RecordScissors(std::initializer_list<VkRect2D> const &scissors) const {
  vkCmdSetScissor(buffer, 0, scissors.size(), scissors.begin());
}

template <std::integral T>
inline void DrawCallRecorder::RecordIndexedDraw(Buffer<T> const &indexBuffer, uint32_t indexCount,
                                                uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                                                uint32_t firstInstance) const {
  RecordIndexBufferBind(indexBuffer);
  vkCmdDrawIndexed(RenderPassRecorder::buffer, static_cast<uint32_t>(indexBuffer.Size()), 1, 0, 0, 0);
}

template <typename T_GPU>
inline void DrawCallRecorder::RecordVertexBufferBind(std::span<VertexBufferBinding<T_GPU>> const &bindings) const {
  std::vector<VkDeviceSize> offsets{};
  offsets.reserve(bindings.size());
  std::vector<VkBuffer> buffers{};
  buffers.reserve(bindings.size());
  std::for_each(bindings.begin(), bindings.end(), [&offsets, &buffers](VertexBufferBinding<T_GPU> const &binding) {
    buffers.push_back(binding.buffer.GetBuffer());
    offsets.push_back(binding.offset);
  });

  vkCmdBindVertexBuffers(RenderPassRecorder::buffer, 0, bindings.size(), buffers.data(), offsets.data());
}

template <std::integral T>
inline void DrawCallRecorder::RecordIndexBufferBind(Buffer<T> const &indexBuffer, VkDeviceSize offset) const {
  vkCmdBindIndexBuffer(RenderPassRecorder::buffer, indexBuffer.GetBuffer(), offset, IndexType<T>::type);
}

inline void CommandRecorder::RecordWithBoundPipeline(Graphics::Pipeline const &pipeline, VkPipelineBindPoint bindPoint,
                                                     MaterialBind auto const &bind) const {
  vkCmdBindPipeline(buffer, bindPoint /* TODO: Take from pipeline directly? */, pipeline.pipeline);
  bind(MaterialBinder(buffer, pipeline.layout, bindPoint));
}

inline void RenderPassRecorder::RecordWithBoundPipeline(Graphics::Pipeline const &pipeline,
                                                        VkPipelineBindPoint bindPoint,
                                                        DrawCall auto const &drawCall) const {
  vkCmdBindPipeline(buffer, bindPoint /* TODO: Take from pipeline directly? */, pipeline.pipeline);
  drawCall(DrawCallRecorder(buffer, pipeline.layout, bindPoint));
}

inline void MaterialBinder::RecordDescriptorBind(std::span<VkDescriptorSet const> const &descriptors) const {
  vkCmdBindDescriptorSets(buffer, usedBindpoint, boundLayout, 0, descriptors.size(), descriptors.data(), 0, nullptr);
}

template <>
inline void MaterialBinder::RecordPushConstantSet<PushConstantsAggregate>(PushConstantsAggregate const &constants,
                                                                          VkShaderStageFlags stage) const {
  vkCmdPushConstants(buffer, boundLayout, stage, 0, constants.Size(), constants.Data());
}
template <typename T>
inline void MaterialBinder::RecordPushConstantSet(T const &constants, VkShaderStageFlags stage) const {
  vkCmdPushConstants(buffer, boundLayout, stage, 0, sizeof(T), &constants);
}

inline void CommandRecorder::RenderPassBuilder::As(RenderPassDefinition auto const &definition) {
  {
    if (!underConstruction.drawImages.size() && !underConstruction.depthImage) {
      ENGINE_WARNING("Render pass without target images defined, skipping pass.");
      return;
    }

    if (!underConstruction.offset) {
      underConstruction.offset = &Maths::Dimension2::Zero;
    }

    if (!underConstruction.extent) {
      if (!underConstruction.depthImage) {
        ENGINE_WARNING("No extent specified for render pass, and deduction was not possible; skipping pass.");
        return;
      }
      underConstruction.extent = &underConstruction.depthImage->GetExtent();
    }

    parentRecorder.RecordRenderPass(underConstruction.drawImages, *underConstruction.depthImage,
                                    underConstruction.depthBufferLoadOp, *underConstruction.extent,
                                    *underConstruction.offset, definition);
  }
}

} // namespace Engine::Graphics
