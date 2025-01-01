#pragma once

#include "Buffer.h"
#include "Image.h"
#include "VulkanUtil.h"

namespace Engine::Graphics {

class BufferCopyCommand : public Command {
  VkBuffer src;
  VkBuffer dst;
  size_t srcOffset; // In bytes
  size_t dstOffset; // In bytes
  size_t size;      // In bytes
public:
  BufferCopyCommand(VkBuffer source, VkBuffer destination, size_t size, size_t sourceOffset = 0,
                    size_t destinationOffset = 0)
      : src(source), dst(destination), srcOffset(sourceOffset), dstOffset(destinationOffset), size(size) {}
  void QueueExecution(VkCommandBuffer const &queue) const;
};

class BufferToImageCopyCommand : public Command {
  VkBuffer src;
  VkImage dst;
  size_t srcOffset;     // In bytes
  VkOffset3D dstOffset; // In bytes
  VkExtent3D dstExtent; // In bytes
  vkutil::PipelineBarrierCommand const *imageTransition;

public:
  BufferToImageCopyCommand(VkBuffer source, VkImage destination, VkExtent3D destinationExtent,
                           vkutil::PipelineBarrierCommand const *imageTransition, size_t sourceOffset = 0,
                           VkOffset3D destinationOffset = {0, 0, 0})
      : src(source), dst(destination), srcOffset(sourceOffset), dstOffset(destinationOffset),
        dstExtent(destinationExtent), imageTransition(imageTransition) {}
  void QueueExecution(VkCommandBuffer const &queue) const;
};

class ImageToBufferCopyCommand : public Command {
  VkImage src;
  VkBuffer dst;
  VkOffset3D srcOffset; // In bytes
  VkExtent3D srcExtent; // In bytes
  size_t dstOffset;     // In bytes
public:
  ImageToBufferCopyCommand(VkImage source, VkBuffer destination, VkExtent3D sourceExtent,
                           VkOffset3D sourceOffset = {0, 0, 0}, size_t destinationOffset = 0)
      : src(source), dst(destination), srcOffset(sourceOffset), dstOffset(destinationOffset), srcExtent(sourceExtent) {}
  void QueueExecution(VkCommandBuffer const &queue) const;
};

class GPUMemoryManager {
private:
  GPUMemoryManager() {}

public:
  template <typename T1, typename T2>
  static BufferCopyCommand CopyBufferToBuffer(Buffer<T1> const &source, Buffer<T2> const &destination, size_t size,
                                              size_t sourceOffset = 0, size_t destinationOffset = 0) {
    return BufferCopyCommand(source.buffer, destination.buffer, size, sourceOffset, destinationOffset);
  }
  template <typename T1, uint8_t D>
  // Image can't be const because it needs to be transitioned
  static BufferToImageCopyCommand *
  CopyBufferToImage(Buffer<T1> const &source, Image<D> &destination, Maths::Dimension<D> destinationExtent,
                    size_t sourceOffset = 0, Maths::Dimension<D> destinationOffset = Maths::Dimension<D>::Zero()) {
    return new BufferToImageCopyCommand(source.buffer, destination.image, vkutil::DimensionToExtent(destinationExtent),
                                        destination.Transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL), sourceOffset,
                                        vkutil::DimensionToOffset(destinationOffset));
  }
};

// Implementations of commands
inline void BufferCopyCommand::QueueExecution(VkCommandBuffer const &queue) const {
  VkBufferCopy copy{.srcOffset = srcOffset, .dstOffset = dstOffset, .size = size};

  vkCmdCopyBuffer(queue, src, dst, 1, &copy);
}

inline void BufferToImageCopyCommand::QueueExecution(VkCommandBuffer const &queue) const {
  imageTransition->QueueExecution(queue);
  delete imageTransition;
  VkBufferImageCopy copy{.bufferOffset = srcOffset,
                         .bufferRowLength = 0,
                         .bufferImageHeight = 0,
                         .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                              .mipLevel = 0,
                                              .baseArrayLayer = 0,
                                              .layerCount = 1},
                         .imageOffset = dstOffset,
                         .imageExtent = dstExtent};

  vkCmdCopyBufferToImage(queue, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
}

inline void ImageToBufferCopyCommand::QueueExecution(VkCommandBuffer const &queue) const {
  VkBufferImageCopy copy{.bufferOffset = dstOffset,
                         .bufferRowLength = 0,
                         .bufferImageHeight = 0,
                         .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                              .mipLevel = 0,
                                              .baseArrayLayer = 0,
                                              .layerCount = 1},
                         .imageOffset = srcOffset,
                         .imageExtent = srcExtent};

  vkCmdCopyImageToBuffer(queue, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst, 1, &copy);
}

} // namespace Engine::Graphics
