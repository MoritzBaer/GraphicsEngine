#pragma once

#include "MemoryAllocator.h"
#include "Util/DeletionQueue.h"
#include "Util/Macros.h"
#include "Window.h"
#include "backends/imgui_impl_vulkan.h"
#include "vulkan/vulkan.h"
#include <optional>
#include <vector>

namespace Engine::Graphics {
struct SwapchainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR presentationSurface);

class InstanceManager {

  VkInstance vulkanInstance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkSurfaceKHR surface;
  VkPhysicalDevice gpu;
  VkDevice graphicsHandler;

  void CreateInstance(const char *applicationName);
#ifndef NDEBUG
  void EnableValidationLayers();
  void SetupDebugMessenger();
#endif
  void PickPhysicalDevice();
  void CreateLogicalDevice();

public:
  InstanceManager(const char *appName, Window const *surfaceWindow);
  ~InstanceManager();

  inline SwapchainSupportDetails GetSwapchainSupport() const { return QuerySwapchainSupport(gpu, surface); }
  uint32_t GetGraphicsFamily() const;
  uint32_t GetPresentFamily() const;
  void GetSwapchainImages(VkSwapchainKHR const &swapchain, std::vector<VkImage> &images) const;
  inline void GetGraphicsQueue(VkQueue *queue) const {
    vkGetDeviceQueue(graphicsHandler, GetGraphicsFamily(), 0, queue);
  }

  inline void GetPresentQueue(VkQueue *queue) const { vkGetDeviceQueue(graphicsHandler, GetPresentFamily(), 0, queue); }

  void FillImGUIInitInfo(ImGui_ImplVulkan_InitInfo &initInfo) const;
  inline void CreateMemoryAllocator(MemoryAllocator &allocator) const {
    allocator.Create(gpu, graphicsHandler, vulkanInstance);
  }

  bool SupportsFormat(VkPhysicalDeviceImageFormatInfo2 const &formatInfo) const;

  // Create vulkan objects
  void CreateSwapchain(
      VkSurfaceFormatKHR const &surfaceFormat, VkPresentModeKHR const &presentMode, VkExtent2D const &extent,
      uint32_t const &imageCount, VkSwapchainKHR const &oldSwapchain,
      VkSwapchainKHR *swapchain) const; // Maybe allow caller to choose every value but surface at some point
  void CreateImageView(VkImageViewCreateInfo const *createInfo, VkImageView *view) const;
  void CreateCommandPool(VkCommandPoolCreateInfo const *createInfo, VkCommandPool *commandPool) const;
  void CreateSemaphore(VkSemaphoreCreateInfo const *createInfo, VkSemaphore *semaphore) const;
  void CreateFence(VkFenceCreateInfo const *createInfo, VkFence *fence) const;
  void CreateShaderModule(VkShaderModuleCreateInfo const *createInfo, VkShaderModule *shaderModule) const;
  void CreateDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo const *createInfo,
                                 VkDescriptorSetLayout *layout) const;
  void CreateDescriptorPool(VkDescriptorPoolCreateInfo const *createInfo, VkDescriptorPool *descriptorPool) const;
  void CreatePipelineLayout(VkPipelineLayoutCreateInfo const *createInfo, VkPipelineLayout *layout) const;
  void CreateComputePipelines(std::vector<VkComputePipelineCreateInfo> const &createInfos, VkPipeline *pipelines) const;
  inline void CreateComputePipeline(VkComputePipelineCreateInfo const &createInfo, VkPipeline *pipeline) const {
    CreateComputePipelines({createInfo}, pipeline);
  }
  void CreateGraphicsPipelines(std::vector<VkGraphicsPipelineCreateInfo> const &createInfos,
                               VkPipeline *pipelines) const;
  inline void CreateGraphicsPipeline(VkGraphicsPipelineCreateInfo const &createInfo, VkPipeline *pipeline) const {
    CreateGraphicsPipelines({createInfo}, pipeline);
  }
  void CreateSampler(VkSamplerCreateInfo const *createInfo, VkSampler *sampler) const;

  // Destroy vulkan objects
  inline void DestroySwapchain(VkSwapchainKHR const &swapchain) const {
    vkDestroySwapchainKHR(graphicsHandler, swapchain, nullptr);
  }
  inline void DestroyImageView(VkImageView const &view) const { vkDestroyImageView(graphicsHandler, view, nullptr); }
  inline void DestroyCommandPool(VkCommandPool const &pool) const {
    vkDestroyCommandPool(graphicsHandler, pool, nullptr);
  }
  inline void DestroySemaphore(VkSemaphore const &semaphore) const {
    vkDestroySemaphore(graphicsHandler, semaphore, nullptr);
  }
  inline void DestroyFence(VkFence const &fence) const { vkDestroyFence(graphicsHandler, fence, nullptr); }
  inline void DestroyShaderModule(VkShaderModule const &shaderModule) const {
    vkDestroyShaderModule(graphicsHandler, shaderModule, nullptr);
  }
  inline void DestroyDescriptorSetLayout(VkDescriptorSetLayout const &layout) const {
    vkDestroyDescriptorSetLayout(graphicsHandler, layout, nullptr);
  }
  inline void DestroyDescriptorPool(VkDescriptorPool const &pool) const {
    vkDestroyDescriptorPool(graphicsHandler, pool, nullptr);
  }
  inline void DestroyPipelineLayout(VkPipelineLayout const &layout) const {
    vkDestroyPipelineLayout(graphicsHandler, layout, nullptr);
  }
  inline void DestroyPipeline(VkPipeline const &pipeline) const {
    vkDestroyPipeline(graphicsHandler, pipeline, nullptr);
  }
  inline void DestroySampler(VkSampler const &sampler) const { vkDestroySampler(graphicsHandler, sampler, nullptr); }

  // Allocate vulkan memory
  void AllocateCommandBuffers(VkCommandBufferAllocateInfo const *allocInfo, VkCommandBuffer *commandBuffers) const;
  VkResult AllocateDescriptorSets(std::vector<VkDescriptorSetLayout> const &layouts,
                                  VkDescriptorPool const &descriptorPool, VkDescriptorSet *descriptorSets) const;
  inline VkResult AllocateDescriptorSets(VkDescriptorSetLayout const &layout, VkDescriptorPool const &descriptorPool,
                                         VkDescriptorSet *descriptorSet) const {
    return AllocateDescriptorSets(std::vector<VkDescriptorSetLayout>{layout}, descriptorPool, descriptorSet);
  }

  // Free vulkan memory
  inline void FreeCommandBuffers(VkCommandPool const &commandPool, VkCommandBuffer const *buffers,
                                 uint32_t bufferCount = 1) const {
    vkFreeCommandBuffers(graphicsHandler, commandPool, bufferCount, buffers);
  }

  // Vulkan synchronization
  void WaitForFences(VkFence const *fences, uint32_t fenceCount = 1, bool waitForAll = true,
                     uint32_t timeout = 1000000000) const;
  void ResetFences(VkFence const *fences, uint32_t fenceCount = 1) const;
  inline void WaitUntilDeviceIdle() const { vkDeviceWaitIdle(graphicsHandler); }

  // Swapchain handling
  uint32_t GetNextSwapchainImageIndex(VkResult &acquisitionResult, VkSwapchainKHR const &swapchain,
                                      VkSemaphore const &semaphore = VK_NULL_HANDLE,
                                      VkFence const &fence = VK_NULL_HANDLE, uint32_t timeout = 1000000000) const;
  inline uint32_t GetNextSwapchainImageIndex(VkSwapchainKHR const &swapchain,
                                             VkSemaphore const &semaphore = VK_NULL_HANDLE,
                                             VkFence const &fence = VK_NULL_HANDLE,
                                             uint32_t timeout = 1000000000) const {
    VkResult _;
    return GetNextSwapchainImageIndex(_, swapchain, semaphore, fence, timeout);
  }

  // Miscellaneous vulkan wrappers
  inline void ClearDescriptorPool(VkDescriptorPool const &pool, VkDescriptorPoolResetFlags flags = 0) const {
    vkResetDescriptorPool(graphicsHandler, pool, flags);
  }
  inline void UpdateDescriptorSets(std::vector<VkWriteDescriptorSet> const &descriptorSets) const {
    vkUpdateDescriptorSets(graphicsHandler, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0,
                           nullptr);
  }
  inline void UpdateDescriptorSets(VkWriteDescriptorSet const &descriptorSet) const {
    UpdateDescriptorSets(std::vector<VkWriteDescriptorSet>{descriptorSet});
  }
  inline VkDeviceAddress GetBufferDeviceAddress(VkBufferDeviceAddressInfo const *bufferInfo) const {
    return vkGetBufferDeviceAddress(graphicsHandler, bufferInfo);
  }
};

} // namespace Engine::Graphics
