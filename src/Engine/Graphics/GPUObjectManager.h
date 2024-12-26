#pragma once

#include "GPUDispatcher.h"
#include "InstanceManager.h"
#include "MemoryAllocator.h"

#include "AllocatedMesh.h"
#include "Buffer.h"
#include "Texture.h"

namespace Engine::Graphics {

class GPUObjectManager {
  InstanceManager const &instanceManager;
  MemoryAllocator
#ifdef NDEBUG
      const
#endif
          &memoryAllocator;
  CommandQueue dispatcherQueue;
  GPUDispatcher dispatcher;

public:
  GPUObjectManager(InstanceManager &instanceManager, MemoryAllocator &memoryAllocator)
      : instanceManager(instanceManager), memoryAllocator(memoryAllocator), dispatcherQueue(CreateCommandQueue()),
        dispatcher(instanceManager, dispatcherQueue) {}
  ~GPUObjectManager() { DestroyCommandQueue(dispatcherQueue); }

  template <uint8_t D>
  inline Image<D> CreateImage(VkImage image, Maths::Dimension<D> const &imageSize, VkFormat imageFormat,
                              VkImageLayout currentLayout, VkImageAspectFlags aspectMask, uint32_t mipLevels = 1,
                              uint32_t arrayLayers = 1) const;

  template <uint8_t D>
  inline AllocatedImage<D> CreateAllocatedImage(VkFormat format, Maths::Dimension<D> const &imageSize,
                                                VkImageUsageFlags usage, VkImageAspectFlags aspectMask,
                                                uint32_t mipLevels = 1, uint32_t arrayLayers = 1,
                                                VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT
#ifndef NDEBUG
                                                ,
                                                char const *label = nullptr
#endif
  ) const;

  template <uint8_t D>
  inline Texture<D> CreateTexture(Maths::Dimension<D> const &imageSize, VkFilter magFilter = VK_FILTER_LINEAR,
                                  VkFilter minFilter = VK_FILTER_LINEAR, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
                                  bool mipped = true, VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT)
#ifdef NDEBUG
      const
#endif
      ;
  template <uint8_t D, typename T>
  inline Texture<D> CreateTexture(Maths::Dimension<D> const &imageSize, T const *data,
                                  VkFilter magFilter = VK_FILTER_LINEAR, VkFilter minFilter = VK_FILTER_LINEAR,
                                  VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, bool mipped = true,
                                  VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT)
#ifdef NDEBUG
      const
#endif
      ;

  template <uint8_t D, typename T>
  inline void SetPixels(Texture<D> &target, T const *data, Maths::Dimension<D> dimension)
#ifdef NDEBUG
      const
#endif
      ;
  template <uint8_t D, typename T>
  inline void SetPixels(Texture<D> &target, T const *data)
#ifdef NDEBUG
      const
#endif
  {
    SetPixels(target, data, target.GetExtent());
  }

  template <typename T>
  Buffer<T> CreateBuffer(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage
#ifndef NDEBUG
                         ,
                         char const *label = nullptr
#endif
  )
#ifdef NDEBUG
      const
#endif
      ;
  template <typename T>
  Buffer<T> CreateBuffer(T const *data, size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage
#ifndef NDEBUG
                         ,
                         char const *label = nullptr
#endif
  )
#ifdef NDEBUG
      const
#endif
  {
    auto buffer = CreateBuffer<T>(size, usage, memoryUsage, label);
    buffer.SetData(data, size);
    return buffer;
  }

  template <typename T>
  Buffer<T> CreateBuffer(T const &data, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage
#ifndef NDEBUG
                         ,
                         char const *label = nullptr
#endif
  )
#ifdef NDEBUG
      const
#endif
  {
    return CreateBuffer(&data, 1, usage, memoryUsage, label);
  } // namespace Engine::Graphics

  template <uint8_t D> inline void DestroyImage(Image<D> const &image) const {
    instanceManager.DestroyImageView(image.imageView);
  }

  template <uint8_t D>
  inline void DestroyAllocatedImage(AllocatedImage<D> const &image)
#ifdef NDEBUG
      const
#endif
  {
    DestroyImage(image);
    memoryAllocator.DestroyImage(image.image, image.allocation);
  } // namespace Engine::Graphics

  template <uint8_t D>
  inline void DestroyTexture(Texture<D> const &texture)
#ifdef NDEBUG
      const
#endif
  {
    instanceManager.DestroySampler(texture.sampler);
    DestroyAllocatedImage(texture);
  }

  template <typename T>
  inline void DestroyBuffer(Buffer<T> const &buffer)
#ifdef NDEBUG
      const
#endif
  {
    memoryAllocator.DestroyBuffer(buffer.buffer, buffer.allocation);
  }

  template <typename T> inline VkDeviceAddress GetDeviceAddresss(Buffer<T> buffer) const;

  template <typename T_CPU, typename T_GPU>
  inline AllocatedMesh AllocateMesh(MeshT<T_CPU> const &mesh)
#ifdef NDEBUG
      const
#endif
      ;

  inline void DeallocateMesh(AllocatedMesh *mesh)
#ifdef NDEBUG
      const
#endif
  {
    memoryAllocator.DestroyBuffer(mesh->indexBuffer.buffer, mesh->indexBuffer.allocation);
    memoryAllocator.DestroyBuffer(mesh->vertexBuffer->GetBuffer(), mesh->vertexBuffer->GetAllocation());
    delete mesh->vertexBuffer;
  }

  inline CommandQueue CreateCommandQueue() const;
  inline void DestroyCommandQueue(CommandQueue const &queue) const;
};

inline CommandQueue GPUObjectManager::CreateCommandQueue() const {
  VkCommandPoolCreateInfo commandPoolInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = instanceManager.GetGraphicsFamily(),
  };

  VkCommandPool commandPool;

  instanceManager.CreateCommandPool(&commandPoolInfo, &commandPool);

  VkCommandBufferAllocateInfo commandBufferInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };

  VkCommandBuffer mainBuffer;

  instanceManager.AllocateCommandBuffers(&commandBufferInfo, &mainBuffer);

  return CommandQueue(commandPool, mainBuffer);
}

inline void GPUObjectManager::DestroyCommandQueue(CommandQueue const &queue) const {
  instanceManager.FreeCommandBuffers(queue.commandPool, &queue.mainBuffer);
  instanceManager.DestroyCommandPool(queue.commandPool);
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

  instanceManager.CreateImageView(&imageViewCreateInfo, &imageView);
  return Image<D>(image, imageView, imageSize, imageFormat, currentLayout);
}

template <uint8_t D>
inline AllocatedImage<D> GPUObjectManager::CreateAllocatedImage(VkFormat format, Maths::Dimension<D> const &imageSize,
                                                                VkImageUsageFlags usage, VkImageAspectFlags aspectMask,
                                                                uint32_t mipLevels, uint32_t arrayLayers,
                                                                VkSampleCountFlagBits msaaSamples
#ifndef NDEBUG
                                                                ,
                                                                char const *label
#endif
) const {
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
  memoryAllocator.CreateImage(&imageCreateInfo, &im, &allocation, label);
  return AllocatedImage<D>(
      CreateImage(im, imageSize, format, VK_IMAGE_LAYOUT_UNDEFINED, aspectMask, mipLevels, arrayLayers), allocation);
}

template <uint8_t D>
inline Texture<D> GPUObjectManager::CreateTexture(Maths::Dimension<D> const &imageSize, VkFilter magFilter,
                                                  VkFilter minFilter, VkFormat format, bool mipped,
                                                  VkSampleCountFlagBits msaaSamples)
#ifdef NDEBUG
    const
#endif
{
  VkSamplerCreateInfo samplerInfo{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .magFilter = magFilter, .minFilter = minFilter};

  VkSampler sampler;
  instanceManager.CreateSampler(&samplerInfo, &sampler);
  return Texture<D>(CreateAllocatedImage(
                        format, imageSize,
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                        VK_IMAGE_ASPECT_COLOR_BIT,
                        (mipped ? static_cast<uint32_t>(std::floor(std::log2(imageSize.maxEntry()))) : 0) + 1, 1,
                        msaaSamples
#ifndef NDEBUG
                        ,
                        "TEXTURE"
#endif
                        ),
                    sampler);
}

template <uint8_t D, typename T>
inline Texture<D> GPUObjectManager::CreateTexture(Maths::Dimension<D> const &imageSize, T const *data,
                                                  VkFilter magFilter, VkFilter minFilter, VkFormat format, bool mipped,
                                                  VkSampleCountFlagBits msaaSamples)
#ifdef NDEBUG
    const
#endif
{
  auto texture = CreateTexture(imageSize, magFilter, minFilter, format, mipped, msaaSamples);
  SetPixels(texture, data, imageSize);
  return texture;
}

template <uint8_t D, typename T>
inline void GPUObjectManager::SetPixels(Texture<D> &target, T const *data, Maths::Dimension<D> dimension)
#ifdef NDEBUG
    const
#endif
{
  Buffer<T> pixelBuffer =
      CreateBuffer(data, dimension.Volume(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

  auto copy = GPUMemoryManager::CopyBufferToImage(pixelBuffer, target, dimension);
  auto transition = target.Image<D>::Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  CompositeCommand copyAndTransition{&copy, &transition};
  dispatcher.Dispatch(&copyAndTransition);

  DestroyBuffer(pixelBuffer);
}

template <typename T>
Buffer<T> GPUObjectManager::CreateBuffer(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage
#ifndef NDEBUG
                                         ,
                                         char const *label
#endif
)
#ifdef NDEBUG
    const
#endif
{
  VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size * sizeof(T), .usage = usage};
  VkBuffer buffer;

  VmaAllocationCreateInfo allocInfo{.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT, .usage = memoryUsage};
  VmaAllocation allocation;

  VmaAllocationInfo info;

  memoryAllocator.CreateBuffer(&bufferInfo, &allocInfo, &buffer, &allocation, &info, label);

  return Buffer<T>(buffer, allocation, info, size);
}

template <typename T> inline VkDeviceAddress GPUObjectManager::GetDeviceAddresss(Buffer<T> buffer) const {
  auto info = buffer.GetDeviceAddresssInfo();
  return instanceManager.GetBufferDeviceAddress(&info);
}

template <typename T_CPU, typename T_GPU>
inline AllocatedMesh GPUObjectManager::AllocateMesh(MeshT<T_CPU> const &mesh)
#ifdef NDEBUG
    const
#endif
{
  Buffer<T_GPU> vertexBuffer;
  Buffer<uint32_t> indexBuffer;
  indexBuffer =
      CreateBuffer<uint32_t>(mesh.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             // TODO: Replace with VMA_MEMORY_USAGE_AUTO + flags
                             VMA_MEMORY_USAGE_GPU_ONLY
#ifndef NDEBUG
                             ,
                             "INDEX_BUFFER"
#endif
      );
  vertexBuffer = CreateBuffer<VertexFormat>(
      mesh.vertices.size(),
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY
#ifndef NDEBUG
      ,
      "VERTEX_BUFFER"
#endif
  );
  auto vertexBufferAddress = GetDeviceAddresss(vertexBuffer);

  auto uploadReadyVertices(mesh.ReformattedVertices<VertexFormat>());

  Buffer<uint8_t> stagingBuffer = CreateBuffer<uint8_t>(vertexBuffer.PhysicalSize() + indexBuffer.PhysicalSize(),
                                                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY
#ifndef NDEBUG
                                                        ,
                                                        "STAGING_BUFFER"
#endif
  );

  void *data = stagingBuffer.GetMappedData();
  memcpy(data, uploadReadyVertices.data(), vertexBuffer.PhysicalSize());
  memcpy((char *)data + vertexBuffer.PhysicalSize(), mesh.indices.data(), indexBuffer.PhysicalSize());

  auto unstage = UnstageMeshCommand(stagingBuffer, vertexBuffer, indexBuffer);
  dispatcher.Dispatch(&unstage);
  DestroyBuffer(stagingBuffer);
  return AllocatedMesh(new VertexBufferT<T_GPU>(vertexBuffer), indexBuffer, vertexBufferAddress);
}

} // namespace Engine::Graphics