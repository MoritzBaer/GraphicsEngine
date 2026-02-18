#pragma once

#include "Buffer.h"
#include "Command.h"
#include "Graphics/AllocatedMesh.h"
#include "Graphics/Image.h"
#include "Graphics/Material.h"
#include "Maths/Dimension.h"
#include "Util/DeletionQueue.h"
#include "vulkan/vulkan.h"
#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <span>
#include <vulkan/vulkan_core.h>

namespace Engine::Graphics {

class GPUObjectManager;

template <typename T_GPU> struct VertexBufferBinding {
  VertexBufferT<T_GPU> const &buffer;
  VkDeviceSize offset;
};

class CommandRecorder {
  VkCommandBuffer const &buffer;

public:
  CommandRecorder(VkCommandBuffer const &buffer) : buffer(buffer) {}

  // +-------- Copy operations --------+

  // Buffer-to-buffer
  template <typename T1, typename T2>
  void RecordCopy(Buffer<T1> const &src, Buffer<T2> const &dst, size_t numBytes, size_t srcOffset = 0,
                  size_t dstOffset = 0) const;
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

  // +----------- Rendering -----------+
  void SetViewports(std::initializer_list<VkViewport> const &viewports) const;
  void SetScissors(std::initializer_list<VkRect2D> const &scissors) const;
  void BeginRendering() const;
  void EndRendering() const;

  template <std::integral T>
  void DrawIndexed(Buffer<T> const &indexBuffer, uint32_t indexCount, uint32_t instanceCount = 1,
                   uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) const;
  template <std::integral T>
  inline void DrawIndexed(Buffer<T> const &indexBuffer, uint32_t instanceCount = 1, uint32_t firstIndex = 0,
                          int32_t vertexOffset = 0, uint32_t firstInstance = 0) const {
    DrawIndexed(indexBuffer, indexBuffer.Size(), instanceCount, firstIndex, vertexOffset, firstInstance);
  }

  template <typename T_GPU>
  void BindVertexBuffers(std::initializer_list<VertexBufferBinding<T_GPU>> const &bindings) const;
  template <typename T_GPU>
  inline void BindVertexBuffers(std::initializer_list<VertexBufferT<T_GPU const &>> const &buffers) const {
    std::vector<VertexBufferBinding<T_GPU>> bindings{};
    std::transform(buffers.begin(), buffers.end(), bindings.back_inserter(), [](VertexBufferT<T_GPU> const &buf) {
      return VertexBufferBinding<T_GPU>{.buffer = buf, .offset = 0};
    });
    BindVertexBuffers(bindings);
  }

  template <std::integral T> void BindIndexBuffer(Buffer<T> const &buffer, VkDeviceSize offset = 0);

  void BindPipeline(Pipeline const &pipeline) const;
  void BindDescriptors(Pipeline const &pipeline, std::initializer_list<VkDescriptorSet> const &descriptors) const;
  template <typename T>
  void PushConstants(T const & constants, Pipeline const & pipeline, VkShaderStageFlags stage) const;

  // +-------- Image operations -------+
  void AddPipelineBarrier(VkDependencyInfo const &dependencies) const;
  template <uint8_t D>
  void ClearColorImage(Image<D> const &image, VkClearColorValue const &clearColour,
                       std::initializer_list<VkImageSubresourceRange> const &subresourceRanges) const;
  template <uint8_t D>
  void Blit(Image<D> const &source, Image<D> const &destination,
            std::initializer_list<VkImageBlit2> const &blitRegions) const;
  
};

class CommandQueue {
  friend class GPUObjectManager;

private:
  VkCommandBuffer mainBuffer;
  VkCommandPool pool;

public:
  CommandQueue(VkCommandBuffer mainBuffer, VkCommandPool pool) : mainBuffer(mainBuffer), pool(pool) {}
  CommandQueue() : CommandQueue(VK_NULL_HANDLE, VK_NULL_HANDLE) {}

  VkCommandBufferSubmitInfo EnqueueCommandSequence(std::span<Command const *> const &commands,
                                                   VkCommandBufferUsageFlags flags = 0) const;
};

class CompositeCommand : public Command {
  std::span<Command const *> commands;

public:
  CompositeCommand(std::span<Command const *> const &commands) : commands(commands) {}
  void QueueExecution(VkCommandBuffer const &queue) const {
    for (Command const *command : commands) {
      command->QueueExecution(queue);
      delete command;
    }
  }
};

} // namespace Engine::Graphics
