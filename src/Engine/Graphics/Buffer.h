#pragma once

#include "CommandQueue.h"
#include "Debug/Logging.h"
#include "InstanceManager.h"
#include "MemoryAllocator.h"
#include "Util/DeletionQueue.h"
#include "Util/Macros.h"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

namespace Engine::Graphics {

class GPUMemoryManager;

template <typename T> class Buffer : public Destroyable {
  VkBuffer buffer;
  VmaAllocation allocation;
  VmaAllocationInfo info;
  size_t size;

  friend class GPUMemoryManager;

public:
  void Create(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
  void Destroy() const;

  // TODO: Decide how to handle buffer destruction in general
  // inline ~Buffer() { Destroy(); }

  Buffer() {}
  Buffer(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) { Create(size, usage, memoryUsage); }
  Buffer(T const *data, size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
    Create(size, usage, memoryUsage);
    SetData(data, size);
  }

  inline VkDeviceAddress GetDeviceAddresss() const {
    VkBufferDeviceAddressInfo deviceAdressInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = buffer};

    return InstanceManager::GetBufferDeviceAddress(&deviceAdressInfo);
  }

  inline void *GetMappedData() const { return info.pMappedData; }
  inline size_t Size() const { return size; }
  inline size_t PhysicalSize() const { return size * sizeof(T); }

  inline void SetData(T const *data, size_t numberOfEntries) const {
    memcpy(info.pMappedData, data, numberOfEntries * sizeof(T));
  }
  inline void SetData(std::vector<T> const &data) const { SetData(data.data(), data.size()); }

  inline void BindAsIndexBuffer(VkCommandBuffer const &commandBuffer) const
    requires(std::integral<T>)
  {
    VkIndexType indexType;
    switch (sizeof(T)) {
    case 2:
      indexType = VK_INDEX_TYPE_UINT16;
      break;
    case 4:
      indexType = VK_INDEX_TYPE_UINT32;
      break;
    default:
      ENGINE_ERROR("Invalid index type!");
    }

    vkCmdBindIndexBuffer(commandBuffer, buffer, 0, indexType);
  }
};

template <typename T> void Buffer<T>::Destroy() const { mainAllocator.DestroyBuffer(buffer, allocation); }

template <typename T> void Buffer<T>::Create(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
  this->size = size;

  VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size * sizeof(T), .usage = usage};

  VmaAllocationCreateInfo allocInfo{.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT, .usage = memoryUsage};

  mainAllocator.CreateBuffer(&bufferInfo, &allocInfo, &buffer, &allocation, &info);
}

} // namespace Engine::Graphics
