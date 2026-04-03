#pragma once

#include "GPUDispatcher.h"
#include "Graphics/CommandQueue.h"
#include "InstanceManager.h"
#include "MemoryAllocator.h"

#include "AllocatedMesh.h"
#include "Buffer.h"
#include "Texture.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

#include "Debug/Logging.h"
#include "Util/Macros.h"

namespace Engine::Graphics {

class GPUObjectManager {
  InstanceManager const *instanceManager;
  MemoryAllocator RELEASE_CONST *memoryAllocator;
  GPUDispatcher dispatcher;

public:
  GPUObjectManager(InstanceManager const *instanceManager, MemoryAllocator *memoryAllocator)
      : instanceManager(instanceManager), memoryAllocator(memoryAllocator), dispatcher(CreateGPUDispatcher()) {}
  ~GPUObjectManager() { DestroyGPUDispatcher(dispatcher); }

  template <uint8_t D>
  inline Image<D> CreateImage(VkImage image, Maths::Dimension<D> const &imageSize, VkFormat imageFormat,
                              VkImageLayout currentLayout, VkImageAspectFlags aspectMask, uint32_t mipLevels = 1,
                              uint32_t arrayLayers = 1) const;

  template <uint8_t D>
  inline Image<D>
  AllocateImage(VkFormat format, Maths::Dimension<D> const &imageSize, VkImageUsageFlags usage,
                VkImageAspectFlags aspectMask, uint32_t mipLevels = 1, uint32_t arrayLayers = 1,
                VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT DEBUG_LABEL_DEFAULT("IMAGE")) RELEASE_CONST;
  inline Image2 CreateDepthBuffer(Maths::Dimension2 const &imageSize,
                                  VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT) RELEASE_CONST {
    return AllocateImage(VK_FORMAT_D32_SFLOAT, imageSize,
                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                             VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                         VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, msaaSamples DEBUG_LABEL_VALUE("DEPTH_BUFFER"));
  }

  template <uint8_t D>
  inline Texture<D> CreateTexture(Image<D> const &image, VkFilter magFilter = VK_FILTER_LINEAR,
                                  VkFilter minFilter = VK_FILTER_LINEAR) const;
  template <uint8_t D>
  inline Texture<D> CreateTexture(Maths::Dimension<D> const &imageSize, VkFilter magFilter = VK_FILTER_LINEAR,
                                  VkFilter minFilter = VK_FILTER_LINEAR, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
                                  bool mipped = true, VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT,
                                  VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                            VK_IMAGE_USAGE_SAMPLED_BIT,
                                  VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT DEBUG_LABEL_DEFAULT("TEXTURE")) RELEASE_CONST;
  template <uint8_t D, typename T>
  inline Texture<D>
  CreateTexture(Maths::Dimension<D> const &imageSize, T const *data, VkFilter magFilter = VK_FILTER_LINEAR,
                VkFilter minFilter = VK_FILTER_LINEAR, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, bool mipped = true,
                VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT,
                VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                          VK_IMAGE_USAGE_SAMPLED_BIT,
                VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT DEBUG_LABEL_DEFAULT("TEXTURE")) RELEASE_CONST;

  template <uint8_t D, typename T>
  inline void SetPixels(Texture<D> &target, T const *data, Maths::Dimension<D> dimension) RELEASE_CONST;
  template <uint8_t D, typename T> inline void SetPixels(Texture<D> &target, T const *data) RELEASE_CONST {
    SetPixels(target, data, target.GetExtent());
  }

  template <typename T>
  Buffer<T> CreateBuffer(size_t size, VkBufferUsageFlags usage,
                         VmaMemoryUsage memoryUsage DEBUG_LABEL_DEFAULT("BUFFER")) RELEASE_CONST;

  template <typename T>
  inline Buffer<T> CreateBuffer(T const *data, size_t size, VkBufferUsageFlags usage,
                                VmaMemoryUsage memoryUsage DEBUG_LABEL_DEFAULT("BUFFER")) RELEASE_CONST {
    auto buffer = CreateBuffer<T>(size, usage, memoryUsage DEBUG_LABEL_REFERENCE);
    buffer.SetData(data, size);
    return buffer;
  }

  template <typename T>
  inline Buffer<T> CreateBuffer(std::vector<T> const &data, VkBufferUsageFlags usage,
                                VmaMemoryUsage memoryUsage DEBUG_LABEL_DEFAULT("BUFFER")) RELEASE_CONST {
    return CreateBuffer<T>(data.data(), data.size(), usage, memoryUsage DEBUG_LABEL_REFERENCE);
  }

  template <typename T>
  Buffer<T> CreateBuffer(T const &data, VkBufferUsageFlags usage,
                         VmaMemoryUsage memoryUsage DEBUG_LABEL_DEFAULT("BUFFER")) RELEASE_CONST {
    return CreateBuffer(&data, 1, usage, memoryUsage DEBUG_LABEL_REFERENCE);
  }

  inline GPUDispatcher CreateGPUDispatcher() const { return GPUDispatcher(instanceManager, CreateCommandQueue()); }

  template <uint8_t D> inline void DestroyImage(Image<D> const &image) const {
    instanceManager->DestroyImageView(image.imageView);
    if (image.allocation != VK_NULL_HANDLE) {
      memoryAllocator->DestroyImage(image.image, image.allocation);
    }
  }

  template <uint8_t D> inline void DestroyTexture(Texture<D> const &texture) RELEASE_CONST {
    instanceManager->DestroySampler(texture.sampler);
    DestroyImage(texture);
  }

  template <typename T> inline void DestroyBuffer(Buffer<T> const &buffer) RELEASE_CONST {
    memoryAllocator->DestroyBuffer(buffer.buffer, buffer.allocation);
  }

  template <typename T> inline VkDeviceAddress GetDeviceAddresss(Buffer<T> buffer) const;

  template <typename T_CPU, typename T_GPU> inline AllocatedMesh AllocateMesh(MeshT<T_CPU> const &mesh) RELEASE_CONST;

  inline void DeallocateMesh(AllocatedMesh *mesh) RELEASE_CONST {
    memoryAllocator->DestroyBuffer(mesh->indexBuffer.buffer, mesh->indexBuffer.allocation);
    memoryAllocator->DestroyBuffer(mesh->vertexBuffer->GetBuffer(), mesh->vertexBuffer->GetAllocation());
    delete mesh->vertexBuffer;
  }

  inline CommandQueue CreateCommandQueue() const;
  inline void DestroyCommandQueue(CommandQueue const &queue) const;
  inline void DestroyGPUDispatcher(GPUDispatcher const &dispatcher) const {
    DestroyCommandQueue(dispatcher.commandQueue);
    instanceManager->DestroyFence(dispatcher.fence);
  }
};

// +-----------------+
// | IMPLEMENTATIONS |
// +-----------------+

inline CommandQueue GPUObjectManager::CreateCommandQueue() const {
  VkCommandPoolCreateInfo commandPoolInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = instanceManager->GetGraphicsFamily(),
  };

  VkCommandPool commandPool;

  instanceManager->CreateCommandPool(&commandPoolInfo, &commandPool);

  VkCommandBufferAllocateInfo commandBufferInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };

  VkCommandBuffer mainBuffer;

  instanceManager->AllocateCommandBuffers(&commandBufferInfo, &mainBuffer);

  return CommandQueue(mainBuffer, commandPool);
}

inline void GPUObjectManager::DestroyCommandQueue(CommandQueue const &queue) const {
  instanceManager->FreeCommandBuffers(queue.pool, &queue.mainBuffer);
  instanceManager->DestroyCommandPool(queue.pool);
}

template <uint8_t D>
inline Image<D> GPUObjectManager::CreateImage(VkImage image, Maths::Dimension<D> const &imageSize, VkFormat imageFormat,
                                              VkImageLayout currentLayout, VkImageAspectFlags aspectMask,
                                              uint32_t mipLevels, uint32_t arrayLayers) const {
  VkImageView imageView;
  VkImageViewCreateInfo imageViewCreateInfo{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                            .image = image,
                                            .viewType = Image<D>::VIEW_TYPE,
                                            .format = imageFormat,
                                            .subresourceRange = {.aspectMask = aspectMask,
                                                                 .baseMipLevel = 0,
                                                                 .levelCount = mipLevels,
                                                                 .baseArrayLayer = 0,
                                                                 .layerCount = arrayLayers}};

  instanceManager->CreateImageView(&imageViewCreateInfo, &imageView);
  return Image<D>(image, imageView, imageSize, imageFormat, currentLayout, aspectMask);
}

template <uint8_t D>
inline Image<D> GPUObjectManager::AllocateImage(VkFormat format, Maths::Dimension<D> const &imageSize,
                                                VkImageUsageFlags usage, VkImageAspectFlags aspectMask,
                                                uint32_t mipLevels, uint32_t arrayLayers,
                                                VkSampleCountFlagBits msaaSamples DEBUG_LABEL) RELEASE_CONST {
  VkImageCreateInfo imageCreateInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = Image<D>::IMAGE_TYPE,
      .format = format,
      .extent = vkutil::DimensionToExtent(imageSize),
      .mipLevels = mipLevels,
      .arrayLayers = arrayLayers,
      .samples = msaaSamples,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = usage,
  };

  VkImage im;
  VmaAllocation allocation;
  memoryAllocator->CreateImage(&imageCreateInfo, &im, &allocation DEBUG_LABEL_REFERENCE);
  auto img = CreateImage(im, imageSize, format, VK_IMAGE_LAYOUT_UNDEFINED, aspectMask, mipLevels, arrayLayers);
  img.allocation = allocation;
  return img;
}

// To use, `image` must have been created with `VK_IMAGE_USAGE_SAMPLED_BIT`
template <uint8_t D>
inline Texture<D> GPUObjectManager::CreateTexture(Image<D> const &image, VkFilter magFilter, VkFilter minFilter) const {

  VkSamplerCreateInfo samplerInfo{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .magFilter = magFilter, .minFilter = minFilter};

  VkSampler sampler;
  instanceManager->CreateSampler(&samplerInfo, &sampler);
  return Texture<D>(image, sampler);
}

template <uint8_t D>
inline Texture<D> GPUObjectManager::CreateTexture(Maths::Dimension<D> const &imageSize, VkFilter magFilter,
                                                  VkFilter minFilter, VkFormat format, bool mipped,
                                                  VkSampleCountFlagBits msaaSamples, VkImageUsageFlags usage,
                                                  VkImageAspectFlags aspectMask DEBUG_LABEL) RELEASE_CONST {
  if (!(usage & VK_IMAGE_USAGE_SAMPLED_BIT)) {
    ENGINE_WARNING("Texture created without the sampled bit set!")
  }
  return CreateTexture(
      AllocateImage(format, imageSize, usage, aspectMask,
                    (mipped ? static_cast<uint32_t>(std::floor(std::log2(imageSize.MaxEntry()))) : 0) + 1, 1,
                    msaaSamples DEBUG_LABEL_REFERENCE),
      magFilter, minFilter);
}

template <uint8_t D, typename T>
inline Texture<D> GPUObjectManager::CreateTexture(Maths::Dimension<D> const &imageSize, T const *data,
                                                  VkFilter magFilter, VkFilter minFilter, VkFormat format, bool mipped,
                                                  VkSampleCountFlagBits msaaSamples, VkImageUsageFlags usage,
                                                  VkImageAspectFlags aspectMask DEBUG_LABEL) RELEASE_CONST {
  auto texture = CreateTexture(imageSize, magFilter, minFilter, format, mipped, msaaSamples, usage, aspectMask DEBUG_LABEL_REFERENCE);
  SetPixels(texture, data, imageSize);
  return texture;
}

template <uint8_t D, typename T>
inline void GPUObjectManager::SetPixels(Texture<D> &target, T const *data,
                                        Maths::Dimension<D> dimension) RELEASE_CONST {
  Buffer<T> pixelBuffer =
      CreateBuffer(data, dimension.Volume(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

  dispatcher.Dispatch([&](CommandRecorder const &recorder) {
    recorder.RecordCopy(pixelBuffer, target);
    recorder.RecordTransition(target, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  });

  DestroyBuffer(pixelBuffer);
}

template <typename T>
Buffer<T> GPUObjectManager::CreateBuffer(size_t size, VkBufferUsageFlags usage,
                                         VmaMemoryUsage memoryUsage DEBUG_LABEL) RELEASE_CONST {
  VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size * sizeof(T), .usage = usage};
  VkBuffer buffer;

  VmaAllocationCreateInfo allocInfo{.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT, .usage = memoryUsage};
  VmaAllocation allocation;

  VmaAllocationInfo info;

  memoryAllocator->CreateBuffer(&bufferInfo, &allocInfo, &buffer, &allocation, &info DEBUG_LABEL_REFERENCE);

  return Buffer<T>(buffer, allocation, info, size);
}

template <typename T> inline VkDeviceAddress GPUObjectManager::GetDeviceAddresss(Buffer<T> buffer) const {
  auto info = buffer.GetDeviceAddresssInfo();
  return instanceManager->GetBufferDeviceAddress(&info);
}

template <typename T_CPU, typename T_GPU>
inline AllocatedMesh GPUObjectManager::AllocateMesh(MeshT<T_CPU> const &mesh) RELEASE_CONST {
  Buffer<T_GPU> vertexBuffer;
  Buffer<uint32_t> indexBuffer;
  indexBuffer =
      CreateBuffer<uint32_t>(mesh.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             // TODO: Replace with VMA_MEMORY_USAGE_AUTO + flags
                             VMA_MEMORY_USAGE_GPU_ONLY DEBUG_LABEL_VALUE("INDEX_BUFFER"));
  vertexBuffer = CreateBuffer<VertexFormat>(mesh.vertices.size(),
                                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                            VMA_MEMORY_USAGE_GPU_ONLY DEBUG_LABEL_VALUE("VERTEX_BUFFER"));
  auto const vertexBufferAddress = GetDeviceAddresss(vertexBuffer);

  auto const uploadReadyVertices = mesh.template ReformattedVertices<VertexFormat>();

  Buffer<uint8_t> stagingBuffer =
      CreateBuffer<uint8_t>(vertexBuffer.PhysicalSize() + indexBuffer.PhysicalSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VMA_MEMORY_USAGE_CPU_ONLY DEBUG_LABEL_VALUE("STAGING_BUFFER"));

  void *data = stagingBuffer.GetMappedData();
  memcpy(data, uploadReadyVertices.data(), vertexBuffer.PhysicalSize());
  memcpy((char *)data + vertexBuffer.PhysicalSize(), mesh.indices.data(), indexBuffer.PhysicalSize());

  dispatcher.Dispatch([&stagingBuffer, &vertexBuffer, &indexBuffer](CommandRecorder const &recorder) {
    recorder.RecordCopy(stagingBuffer, vertexBuffer, vertexBuffer.PhysicalSize(), 0, 0);
    recorder.RecordCopy(stagingBuffer, indexBuffer, indexBuffer.PhysicalSize(), vertexBuffer.PhysicalSize(), 0);
  });
  DestroyBuffer(stagingBuffer);
  return AllocatedMesh(new VertexBufferT<T_GPU>(vertexBuffer), indexBuffer, vertexBufferAddress);
}

} // namespace Engine::Graphics