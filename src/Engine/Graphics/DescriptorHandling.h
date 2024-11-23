#pragma once

#include "Debug/Logging.h"
#include "Image.h"
#include "vulkan/vulkan.h"
#include <algorithm>
#include <deque>
#include <span>

namespace Engine::Graphics {

enum class DescriptorType {

};

class DescriptorLayoutBuilder {
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  InstanceManager const *instanceManager;

public:
  inline DescriptorLayoutBuilder(InstanceManager const *instanceManager)
      : instanceManager(instanceManager), bindings() {}
  inline DescriptorLayoutBuilder &AddBinding(uint32_t binding, VkDescriptorType type);
  inline bool HasBindings() const { return !bindings.empty(); }
  inline void Clear();
  inline VkDescriptorSetLayout Build(VkShaderStageFlags shaderStages);
};

class DescriptorAllocator {
  InstanceManager const *instanceManager;

public:
  struct PoolSizeRatio {
    VkDescriptorType type;
    float ratio;
  };

private:
  inline VkDescriptorPool GetPool();
  inline VkDescriptorPool CreatePool(uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);

  std::vector<PoolSizeRatio> poolRatios;
  std::vector<VkDescriptorPool> fullPools;
  std::vector<VkDescriptorPool> readyPools;
  uint32_t setsPerPool;

public:
  DescriptorAllocator(InstanceManager const *instanceManager)
      : instanceManager(instanceManager), poolRatios(), fullPools(), readyPools(), setsPerPool(-1) {}
  DescriptorAllocator() : DescriptorAllocator(nullptr) {}

  inline void InitPools(uint32_t initialSets, std::span<PoolSizeRatio> poolRatios);
  inline void ClearDescriptors();
  inline void DestroyPools();

  inline VkDescriptorSet Allocate(VkDescriptorSetLayout layout);
};

class DescriptorWriter {
  InstanceManager const *instanceManager;
  std::deque<VkDescriptorImageInfo> imageInfos;
  std::deque<VkDescriptorBufferInfo> bufferInfos;
  std::vector<VkWriteDescriptorSet> writes;

public:
  DescriptorWriter(InstanceManager const *instanceManager)
      : instanceManager(instanceManager), imageInfos(), bufferInfos(), writes() {}
  // Image must be passed as a pointer to allow subclasses substituting
  template <uint8_t N>
  inline void WriteImage(uint32_t binding, Image<N> const &image, VkImageLayout layout, VkDescriptorType type);
  // TODO: Refactor to use Buffer
  inline void WriteBuffer(uint32_t binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);
  inline void Clear();
  inline void UpdateSet(VkDescriptorSet set);
};

DescriptorLayoutBuilder &DescriptorLayoutBuilder::AddBinding(uint32_t binding, VkDescriptorType type) {
  VkDescriptorSetLayoutBinding newBinding{
      .binding = binding,
      .descriptorType = type,
      .descriptorCount = 1,
  };

  bindings.push_back(newBinding);
  return *this;
}

void DescriptorLayoutBuilder::Clear() { bindings.clear(); }

VkDescriptorSetLayout DescriptorLayoutBuilder::Build(VkShaderStageFlags shaderStages) {
  for (auto &binding : bindings) {
    binding.stageFlags |= shaderStages;
  }

  VkDescriptorSetLayoutCreateInfo layoutInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                                             .bindingCount = static_cast<uint32_t>(bindings.size()),
                                             .pBindings = bindings.data()};

  VkDescriptorSetLayout layout;

  instanceManager->CreateDescriptorSetLayout(&layoutInfo, &layout);

  return layout;
}

void DescriptorAllocator::InitPools(uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) {
  this->poolRatios.clear();
  std::transform(poolRatios.begin(), poolRatios.end(), std::back_inserter(this->poolRatios),
                 [](PoolSizeRatio ratio) { return ratio; });
  VkDescriptorPool newPool = CreatePool(maxSets, poolRatios);
  setsPerPool = maxSets * 1.5;
  readyPools.push_back(newPool);
}

void DescriptorAllocator::ClearDescriptors() {
  for (auto pool : readyPools) {
    instanceManager->ClearDescriptorPool(pool);
  }
  auto &im = *instanceManager;
  std::transform(fullPools.begin(), fullPools.end(), std::back_inserter(readyPools), [&im](VkDescriptorPool pool) {
    im.ClearDescriptorPool(pool);
    return pool;
  });
}

void DescriptorAllocator::DestroyPools() {
  for (auto pool : readyPools) {
    instanceManager->DestroyDescriptorPool(pool);
  }
  readyPools.clear();
  for (auto pool : fullPools) {
    instanceManager->DestroyDescriptorPool(pool);
  }
  fullPools.clear();
}

VkDescriptorSet DescriptorAllocator::Allocate(VkDescriptorSetLayout layout) {
  VkDescriptorPool selectedPool = GetPool();

  VkDescriptorSet allocatedSet;
  VkResult res = instanceManager->AllocateDescriptorSets(layout, selectedPool, &allocatedSet);

  if (res == VK_ERROR_OUT_OF_POOL_MEMORY || res == VK_ERROR_FRAGMENTED_POOL) {
    fullPools.push_back(selectedPool);
    selectedPool = GetPool();
    VULKAN_ASSERT(instanceManager->AllocateDescriptorSets(layout, selectedPool, &allocatedSet),
                  "Failed to allocate descriptor set in new pool!")
  }

  readyPools.push_back(selectedPool);

  return allocatedSet;
}

VkDescriptorPool DescriptorAllocator::GetPool() {
  if (!readyPools.empty()) {
    VkDescriptorPool pool = readyPools.back();
    readyPools.pop_back();
    return pool;
  } else {
    VkDescriptorPool newPool = CreatePool(setsPerPool, poolRatios);
    setsPerPool *= 1.5;
    if (setsPerPool > 4092) {
      setsPerPool = 4092;
    }
    return newPool;
  }
}

VkDescriptorPool DescriptorAllocator::CreatePool(uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) {
  std::vector<VkDescriptorPoolSize> poolSizes(poolRatios.size());
  std::transform(poolRatios.begin(), poolRatios.end(), poolSizes.begin(), [maxSets](PoolSizeRatio ratio) {
    return VkDescriptorPoolSize{.type = ratio.type, .descriptorCount = static_cast<uint32_t>(ratio.ratio * maxSets)};
  });

  VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                                                      .maxSets = maxSets,
                                                      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
                                                      .pPoolSizes = poolSizes.data()};

  VkDescriptorPool descriptorPool;
  instanceManager->CreateDescriptorPool(&descriptorPoolCreateInfo, &descriptorPool);
  return descriptorPool;
}

void DescriptorWriter::WriteBuffer(uint32_t binding, VkBuffer buffer, size_t size, size_t offset,
                                   VkDescriptorType type) {
  VkDescriptorBufferInfo &bufferInfo =
      bufferInfos.emplace_back(VkDescriptorBufferInfo{.buffer = buffer, .offset = offset, .range = size});
  writes.push_back(VkWriteDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                        .dstSet = VK_NULL_HANDLE,
                                        .dstBinding = binding,
                                        .descriptorCount = 1,
                                        .descriptorType = type,
                                        .pBufferInfo = &bufferInfo});
}

void DescriptorWriter::Clear() {
  imageInfos.clear();
  bufferInfos.clear();
  writes.clear();
}

void DescriptorWriter::UpdateSet(VkDescriptorSet set) {
  for (auto &write : writes) {
    write.dstSet = set;
  }

  instanceManager->UpdateDescriptorSets(writes);
}

template <uint8_t N>
void DescriptorWriter::WriteImage(uint32_t binding, Image<N> const &image, VkImageLayout layout,
                                  VkDescriptorType type) {

  VkDescriptorImageInfo const &imageInfo = imageInfos.emplace_back(image.BindInDescriptor(layout));

  VkWriteDescriptorSet write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                             .dstSet = VK_NULL_HANDLE,
                             .dstBinding = binding,
                             .descriptorCount = 1,
                             .descriptorType = type,
                             .pImageInfo = &imageInfo};

  writes.push_back(write);
}

} // namespace Engine::Graphics