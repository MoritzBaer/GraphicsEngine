#pragma once

#include "CommandQueue.h"
#include "DescriptorHandling.h"
#include "InstanceManager.h"
#include "MemoryAllocator.h"
#include "Util/DeletionQueue.h"
#include "Util/Macros.h"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

namespace Engine::Graphics {

class GPUMemoryManager;
class GPUObjectManager;

template <uint8_t size> struct IndexTypeFromSize {
  inline static constexpr std::conditional_t<size == 2 || size == 4, VkIndexType, void> value =
      size == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
};

template <typename T> struct IndexType {
  static constexpr VkIndexType value = IndexTypeFromSize<sizeof(T)>::value;
};

template <typename T> class Buffer {
  VkBuffer buffer;
  VmaAllocation allocation;
  VmaAllocationInfo info;
  size_t size;

  friend class GPUMemoryManager;
  friend class GPUObjectManager;

public:
  inline Buffer() {}

  Buffer(VkBuffer buffer, VmaAllocation allocation, VmaAllocationInfo info, size_t size)
      : buffer(buffer), allocation(allocation), info(info), size(size) {}

  inline VkBufferDeviceAddressInfo GetDeviceAddresssInfo() const {
    return {.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = buffer};
  }

  inline VkBuffer GetBuffer() const { return buffer; }
  inline VmaAllocation GetAllocation() const { return allocation; }

  inline void *GetMappedData() const { return info.pMappedData; }
  inline size_t Size() const { return size; }
  inline size_t PhysicalSize() const { return size * sizeof(T); }

  inline void SetData(T const *data, size_t numberOfEntries) const {
    memcpy(info.pMappedData, data, numberOfEntries * sizeof(T));
  }
  inline void SetData(T const &data) const { SetData(&data, 1); }
  inline void SetData(std::vector<T> const &data) const { SetData(data.data(), data.size()); }

  inline void UpdateDescriptor(DescriptorWriter &writer, VkDescriptorSet descriptorSet, uint32_t binding) const {
    writer.WriteBuffer(binding, buffer, PhysicalSize(), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    writer.UpdateSet(descriptorSet);
    writer.Clear();
  }

  inline void BindAsIndexBuffer(VkCommandBuffer const &commandBuffer) const
    requires(std::integral<T>)
  {
    vkCmdBindIndexBuffer(commandBuffer, buffer, 0, IndexType<T>::value);
  }
};

} // namespace Engine::Graphics
