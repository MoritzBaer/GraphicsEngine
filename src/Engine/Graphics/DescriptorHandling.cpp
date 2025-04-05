#include "DescriptorHandling.h"
namespace Engine::Graphics
{
    
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

} // namespace Engine::Graphics