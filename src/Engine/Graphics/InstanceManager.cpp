#include "InstanceManager.h"

#include "GLFW/glfw3.h"
#include "Util/Macros.h"
#include <algorithm>
#include <set>
#include <vector>

#include "Debug/Logging.h"
#include "WindowManager.h"

#include "Debug/Profiling.h"

namespace Engine::Graphics {

// Callback function for validation layers
// TODO: Write nice formatting function for pMessage
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                    void *pUserData) {

  if (pCallbackData->pMessageIdName == nullptr) {
    return VK_FALSE;
  } // Only necessary because RenderDoc doesn't satisfy the Vulkan specification (so says Max)
  // Lol i haven't got a graphics card that i could use renderdoc with anyways

  switch (messageSeverity) {
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    Debug::Logging::PrintError("Validation", pCallbackData->pMessage);
    __debugbreak();
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    Debug::Logging::PrintWarning("Validation", pCallbackData->pMessage);
    break;
  default:
    Debug::Logging::PrintMessage("Validation", pCallbackData->pMessage);
    break;
  }

  return VK_FALSE;
}

// <Proxies for external vulkan functions>

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator,
                                      VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

// </Proxies for external vulkan functions>

const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> requiredExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

VkPhysicalDeviceVulkan13Features requiredFeatures13{
    .synchronization2 = true,
    .dynamicRendering = true,
};

VkPhysicalDeviceVulkan12Features requiredFeatures12{
    .descriptorIndexing = true,
#ifndef COMPILE_FOR_RENDERDOC
    .bufferDeviceAddress = true,
#endif
};

#define IMPL(name) (!required.name || supported.name)

bool SupportsRequiredFeatures12(VkPhysicalDeviceVulkan12Features supported, VkPhysicalDeviceVulkan12Features required) {
  return IMPL(samplerMirrorClampToEdge) && IMPL(drawIndirectCount) && IMPL(storageBuffer8BitAccess) &&
         IMPL(uniformAndStorageBuffer8BitAccess) && IMPL(storagePushConstant8) && IMPL(shaderBufferInt64Atomics) &&
         IMPL(shaderSharedInt64Atomics) && IMPL(shaderFloat16) && IMPL(shaderInt8) && IMPL(descriptorIndexing) &&
         IMPL(shaderInputAttachmentArrayDynamicIndexing) && IMPL(shaderUniformTexelBufferArrayDynamicIndexing) &&
         IMPL(shaderStorageTexelBufferArrayDynamicIndexing) && IMPL(shaderUniformBufferArrayNonUniformIndexing) &&
         IMPL(shaderSampledImageArrayNonUniformIndexing) && IMPL(shaderStorageBufferArrayNonUniformIndexing) &&
         IMPL(shaderStorageImageArrayNonUniformIndexing) && IMPL(shaderInputAttachmentArrayNonUniformIndexing) &&
         IMPL(shaderUniformTexelBufferArrayNonUniformIndexing) &&
         IMPL(shaderStorageTexelBufferArrayNonUniformIndexing) && IMPL(descriptorBindingUniformBufferUpdateAfterBind) &&
         IMPL(descriptorBindingSampledImageUpdateAfterBind) && IMPL(descriptorBindingStorageImageUpdateAfterBind) &&
         IMPL(descriptorBindingStorageBufferUpdateAfterBind) &&
         IMPL(descriptorBindingUniformTexelBufferUpdateAfterBind) &&
         IMPL(descriptorBindingStorageTexelBufferUpdateAfterBind) && IMPL(descriptorBindingUpdateUnusedWhilePending) &&
         IMPL(descriptorBindingPartiallyBound) && IMPL(descriptorBindingVariableDescriptorCount) &&
         IMPL(runtimeDescriptorArray) && IMPL(samplerFilterMinmax) && IMPL(scalarBlockLayout) &&
         IMPL(imagelessFramebuffer) && IMPL(uniformBufferStandardLayout) && IMPL(shaderSubgroupExtendedTypes) &&
         IMPL(separateDepthStencilLayouts) && IMPL(hostQueryReset) && IMPL(timelineSemaphore) &&
         IMPL(bufferDeviceAddress) && IMPL(bufferDeviceAddressCaptureReplay) && IMPL(bufferDeviceAddressMultiDevice) &&
         IMPL(vulkanMemoryModel) && IMPL(vulkanMemoryModelDeviceScope) &&
         IMPL(vulkanMemoryModelAvailabilityVisibilityChains) && IMPL(shaderOutputViewportIndex) &&
         IMPL(shaderOutputLayer) && IMPL(subgroupBroadcastDynamicId);
}

bool SupportsRequiredFeatures13(VkPhysicalDeviceVulkan13Features supported, VkPhysicalDeviceVulkan13Features required) {
  return IMPL(robustImageAccess) && IMPL(inlineUniformBlock) &&
         IMPL(descriptorBindingInlineUniformBlockUpdateAfterBind) && IMPL(pipelineCreationCacheControl) &&
         IMPL(privateData) && IMPL(shaderDemoteToHelperInvocation) && IMPL(shaderTerminateInvocation) &&
         IMPL(subgroupSizeControl) && IMPL(computeFullSubgroups) && IMPL(synchronization2) &&
         IMPL(textureCompressionASTC_HDR) && IMPL(shaderZeroInitializeWorkgroupMemory) && IMPL(dynamicRendering) &&
         IMPL(shaderIntegerDotProduct) && IMPL(maintenance4);
}

// Only true if in debug mode and layers are available
bool enableValidationLayers = false;

std::vector<const char *> GetRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;

  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::vector<const char *> required(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (enableValidationLayers) {
    required.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return required;
}

bool CheckExtensionAvailability(std::vector<const char *> const &requiredExtensions) {
  uint32_t providedExtensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &providedExtensionCount, nullptr);
  std::vector<VkExtensionProperties> providedExtensions(providedExtensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &providedExtensionCount, providedExtensions.data());

  std::vector<const char *> required(requiredExtensions);

#ifndef NDEBUG
  ENGINE_MESSAGE("Supported instance extensions:")
#endif
  for (auto extension : providedExtensions) {
#ifndef NDEBUG
    ENGINE_MESSAGE("\t{}", extension.extensionName)
#endif
    required.erase(std::remove_if(required.begin(), required.end(),
                                  [extension](const char *str) { return strcmp(extension.extensionName, str); }),
                   required.end());
  }

  return required.empty();
}

bool CheckValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  std::vector<const char *> required(validationLayers);

  for (auto layer : availableLayers) {
    required.erase(std::remove_if(required.begin(), required.end(),
                                  [layer](const char *str) { return strcmp(layer.layerName, str); }),
                   required.end());
  }

  return required.empty();
}

void FillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
  // TODO: Allow user to determine what types of layer messages they want to see
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = DebugCallback;
}

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR const &presentationSurface) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  QueueFamilyIndices indices;

  for (int f = 0; f < queueFamilyCount; f++) {
    if (queueFamilies[f].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = f;
    }
    if (presentationSurface != VK_NULL_HANDLE) {
      VkBool32 canPresent;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, f, presentationSurface, &canPresent);
      if (canPresent) {
        indices.presentFamily = f;
      }
    }
    if (indices.graphicsFamily.has_value() &&
        (indices.presentFamily.has_value() || presentationSurface == VK_NULL_HANDLE)) {
      break;
    }
  }

  return indices;
}

InstanceManager::~InstanceManager() {
  vkDestroyDevice(graphicsHandler, nullptr);
  vkDestroySurfaceKHR(vulkanInstance, surface, nullptr);
  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(vulkanInstance, debugMessenger, nullptr);
  }
  vkDestroyInstance(vulkanInstance, nullptr);
}

void InstanceManager::InitVulkan(const char *appName, Window const *surfaceWindow) {
  PROFILE_FUNCTION()
  // Only set up vulkan validation if in debug mode
#ifndef NDEBUG
  if (CheckValidationLayerSupport()) {
    EnableValidationLayers();
  } else {
    ENGINE_WARNING("Some requested validation layers could not be provided.")
  }
#endif
  CreateInstance(appName); // Instance must be created after validation layer availability has been checked
#ifndef NDEBUG
  SetupDebugMessenger();
#endif
  if (surfaceWindow) {
    surfaceWindow->CreateSurfaceOnWindow(vulkanInstance, &surface);
  }
  PickPhysicalDevice();
  CreateLogicalDevice();
}

void InstanceManager::CreateInstance(const char *applicationName) {
  VkApplicationInfo applicationInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                    .pApplicationName = applicationName,
                                    .applicationVersion = VK_MAKE_VERSION(1, 0, 0), // TODO: Make accessible to user
                                    .pEngineName = "TBNgine",
                                    .engineVersion = ENGINE_VERSION,
                                    .apiVersion = VK_API_VERSION_1_3};

  auto requiredExtensions = GetRequiredExtensions();

  if (!CheckExtensionAvailability(requiredExtensions)) {
    ENGINE_ERROR("Some needed GLFW extensions are unavailable!")
  }

  VkInstanceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &applicationInfo,
      .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
      .ppEnabledExtensionNames = requiredExtensions.data(),
  };

  if (enableValidationLayers) {
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    FillDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
    createInfo.pNext = &debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
  }

  VULKAN_ASSERT(vkCreateInstance(&createInfo, nullptr, &vulkanInstance), "Failed to create vulkan Instance!")
}

#ifndef NDEBUG
void InstanceManager::EnableValidationLayers() { enableValidationLayers = true; }

void InstanceManager::SetupDebugMessenger() {
  if (!enableValidationLayers)
    return;
  VkDebugUtilsMessengerCreateInfoEXT createInfo{};

  FillDebugMessengerCreateInfo(createInfo);

  VULKAN_ASSERT(CreateDebugUtilsMessengerEXT(vulkanInstance, &createInfo, nullptr, &debugMessenger),
                "Failed to create debug messenger!")
}
#endif

bool DeviceHasRequiredExtensions(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> required(requiredExtensions.begin(), requiredExtensions.end());

  for (const auto &extension : availableExtensions) {
    required.erase(extension.extensionName);
  }

  return required.empty();
}

uint32_t PhysicalDeviceScore(VkPhysicalDevice device, VkSurfaceKHR const &presentationSurface) {
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);
  VkPhysicalDeviceVulkan13Features vulkan13Features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  VkPhysicalDeviceVulkan12Features vulkan12Features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
                                                    .pNext = &vulkan13Features};
  VkPhysicalDeviceVulkan11Features vulkan11Features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
                                                    .pNext = &vulkan12Features};
  VkPhysicalDeviceFeatures2 deviceFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                                           .pNext = &vulkan11Features};
  vkGetPhysicalDeviceFeatures2(device, &deviceFeatures);

  auto queueFamilies = FindQueueFamilies(device, presentationSurface);

  bool isDedicatedGPU = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
  bool hasGraphicsFamily = queueFamilies.graphicsFamily.has_value();
  bool hasPresentFamily = queueFamilies.presentFamily.has_value();

  bool extensionsSupported = DeviceHasRequiredExtensions(device);

  bool swapchainAdequate = false;
  if (extensionsSupported) {
    SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(device, presentationSurface);
    swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
  }

  bool hasNecessaryFeatures = deviceFeatures.features.geometryShader &&
                              SupportsRequiredFeatures12(vulkan12Features, requiredFeatures12) &&
                              SupportsRequiredFeatures13(vulkan13Features, requiredFeatures13);

  return hasGraphicsFamily * (presentationSurface == VK_NULL_HANDLE ? 1 : hasPresentFamily) * extensionsSupported *
         swapchainAdequate * hasNecessaryFeatures *
         (deviceProperties.limits.maxImageDimension2D + 1000 * isDedicatedGPU);
}

void InstanceManager::CreateLogicalDevice() {
  auto queueFamilies = FindQueueFamilies(gpu, surface);

  float queuePriority = 1.0f; // WARN: Magic number

  std::set<uint32_t> uniqueIndices = {queueFamilies.graphicsFamily.value(), queueFamilies.presentFamily.value()};
  std::vector<VkDeviceQueueCreateInfo> queueInfos{};

  for (auto family : uniqueIndices) {
    VkDeviceQueueCreateInfo queueInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamilies.graphicsFamily.value(),
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };
    queueInfos.push_back(queueInfo);
  }

  VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
      .dynamicRendering = requiredFeatures13.dynamicRendering};

  VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddress{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
      .pNext = &dynamicRenderingFeatures,
      .bufferDeviceAddress = requiredFeatures12.bufferDeviceAddress,
      .bufferDeviceAddressCaptureReplay = requiredFeatures12.bufferDeviceAddressCaptureReplay,
      .bufferDeviceAddressMultiDevice = requiredFeatures12.bufferDeviceAddressMultiDevice};

  VkPhysicalDeviceSynchronization2Features synchronization{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
      .pNext = &bufferDeviceAddress,
      .synchronization2 = requiredFeatures13.synchronization2};

  VkPhysicalDeviceFeatures2 deviceFeatures2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &synchronization, .features = {}};

  VkDeviceCreateInfo deviceInfo{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                .pNext = &deviceFeatures2,
                                .queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size()),
                                .pQueueCreateInfos = queueInfos.data(),
                                .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
                                .ppEnabledExtensionNames = requiredExtensions.data(),
                                .pEnabledFeatures = nullptr};

  if (enableValidationLayers) {
    deviceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    deviceInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    deviceInfo.enabledLayerCount = 0;
  }

  VULKAN_ASSERT(vkCreateDevice(gpu, &deviceInfo, nullptr, &graphicsHandler), "Failed to create logical device!")
}

#ifdef USING_IMGUI
void InstanceManager::FillImGUIInitInfo(ImGui_ImplVulkan_InitInfo &initInfo) const {
  initInfo.Instance = vulkanInstance;
  initInfo.PhysicalDevice = gpu;
  initInfo.Device = graphicsHandler;
  GetGraphicsQueue(&initInfo.Queue);
}
#endif

bool InstanceManager::SupportsFormat(VkPhysicalDeviceImageFormatInfo2 const &formatInfo) const {
  VkImageFormatProperties2 properties = {};
  properties.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;

  return vkGetPhysicalDeviceImageFormatProperties2(gpu, &formatInfo, &properties) == VK_SUCCESS;
}

void InstanceManager::CreateSwapchain(VkSurfaceFormatKHR const &surfaceFormat, VkPresentModeKHR const &presentMode,
                                      VkExtent2D const &extent, uint32_t const &imageCount,
                                      VkSwapchainKHR const &oldSwapchain, VkSwapchainKHR *swapchain) const {
  auto details = QuerySwapchainSupport(gpu, surface);
  VkSwapchainCreateInfoKHR swapchainInfo{.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                         .surface = surface,
                                         .minImageCount = imageCount,
                                         .imageFormat = surfaceFormat.format,
                                         .imageColorSpace = surfaceFormat.colorSpace,
                                         .imageExtent = extent,
                                         .imageArrayLayers = 1,
                                         .imageUsage =
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                         .preTransform = details.capabilities.currentTransform,
                                         .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                         .presentMode = presentMode,
                                         .clipped = VK_TRUE,
                                         .oldSwapchain = oldSwapchain};

  QueueFamilyIndices indices = FindQueueFamilies(gpu, surface);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchainInfo.queueFamilyIndexCount = 2;
    swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.queueFamilyIndexCount = 0;     // Optional
    swapchainInfo.pQueueFamilyIndices = nullptr; // Optional
  }

  VULKAN_ASSERT(vkCreateSwapchainKHR(graphicsHandler, &swapchainInfo, nullptr, swapchain),
                "Failed to create swapchain!")
}

void InstanceManager::GetSwapchainImages(VkSwapchainKHR const &swapchain, std::vector<VkImage> &images) const {
  uint32_t imageCount;
  vkGetSwapchainImagesKHR(graphicsHandler, swapchain, &imageCount, nullptr);
  images.resize(imageCount);
  vkGetSwapchainImagesKHR(graphicsHandler, swapchain, &imageCount, images.data());
}

void InstanceManager::CreateImageView(VkImageViewCreateInfo const *createInfo, VkImageView *view) const {
  VULKAN_ASSERT(vkCreateImageView(graphicsHandler, createInfo, nullptr, view), "Failed to create image view!")
}

void InstanceManager::CreateCommandPool(VkCommandPoolCreateInfo const *createInfo, VkCommandPool *commandPool) const {
  VULKAN_ASSERT(vkCreateCommandPool(graphicsHandler, createInfo, nullptr, commandPool),
                "Failed to create command pool!")
}

void InstanceManager::CreateSemaphore(VkSemaphoreCreateInfo const *createInfo, VkSemaphore *semaphore) const {
  VULKAN_ASSERT(vkCreateSemaphore(graphicsHandler, createInfo, nullptr, semaphore), "Failed to create semaphore!")
}

void InstanceManager::CreateFence(VkFenceCreateInfo const *createInfo, VkFence *fence) const {
  VULKAN_ASSERT(vkCreateFence(graphicsHandler, createInfo, nullptr, fence), "Failed to create fence!")
}

void InstanceManager::CreateShaderModule(VkShaderModuleCreateInfo const *createInfo,
                                         VkShaderModule *shaderModule) const {
  VULKAN_ASSERT(vkCreateShaderModule(graphicsHandler, createInfo, nullptr, shaderModule),
                "Failed to create shader module!")
}

void InstanceManager::CreateDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo const *createInfo,
                                                VkDescriptorSetLayout *layout) const {
  VULKAN_ASSERT(vkCreateDescriptorSetLayout(graphicsHandler, createInfo, nullptr, layout),
                "Failed to create descriptor set layout!")
}

void InstanceManager::CreateDescriptorPool(VkDescriptorPoolCreateInfo const *createInfo,
                                           VkDescriptorPool *descriptorPool) const {
  VULKAN_ASSERT(vkCreateDescriptorPool(graphicsHandler, createInfo, nullptr, descriptorPool),
                "Failed to create descriptor pool!")
}

void InstanceManager::CreatePipelineLayout(VkPipelineLayoutCreateInfo const *createInfo,
                                           VkPipelineLayout *layout) const {
  PROFILE_FUNCTION()
  VULKAN_ASSERT(vkCreatePipelineLayout(graphicsHandler, createInfo, nullptr, layout),
                "Failed to create pipeline layout!")
}

void InstanceManager::CreateComputePipelines(std::vector<VkComputePipelineCreateInfo> const &createInfos,
                                             VkPipeline *pipelines) const {
  PROFILE_FUNCTION()
  VULKAN_ASSERT(vkCreateComputePipelines(graphicsHandler, VK_NULL_HANDLE, static_cast<uint32_t>(createInfos.size()),
                                         createInfos.data(), nullptr, pipelines),
                "Failed to create compute pipelines!")
}

void InstanceManager::CreateGraphicsPipelines(std::vector<VkGraphicsPipelineCreateInfo> const &createInfos,
                                              VkPipeline *pipelines) const {
  VULKAN_ASSERT(vkCreateGraphicsPipelines(graphicsHandler, VK_NULL_HANDLE, static_cast<uint32_t>(createInfos.size()),
                                          createInfos.data(), nullptr, pipelines),
                "Failed to create graphics pipelines!")
}

void InstanceManager::CreateSampler(VkSamplerCreateInfo const *createInfo, VkSampler *sampler) const {
  VULKAN_ASSERT(vkCreateSampler(graphicsHandler, createInfo, nullptr, sampler), "Failed to create sampler!")
}

void InstanceManager::AllocateCommandBuffers(VkCommandBufferAllocateInfo const *allocInfo,
                                             VkCommandBuffer *commandBuffers) const {
    VULKAN_ASSERT(vkAllocateCommandBuffers(graphicsHandler, allocInfo, commandBuffers),
                  "Failed to allocate command buffers!")}

VkResult InstanceManager::AllocateDescriptorSets(std::vector<VkDescriptorSetLayout> const &layouts,
                                                 VkDescriptorPool const &descriptorPool,
                                                 VkDescriptorSet *descriptorSets) const {
  VkDescriptorSetAllocateInfo allocationInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                             .descriptorPool = descriptorPool,
                                             .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
                                             .pSetLayouts = layouts.data()};

  VkResult allocationResult = vkAllocateDescriptorSets(graphicsHandler, &allocationInfo, descriptorSets);
  if (allocationResult != VK_SUCCESS && allocationResult != VK_ERROR_OUT_OF_POOL_MEMORY &&
      allocationResult == VK_ERROR_FRAGMENTED_POOL) {
    ENGINE_ERROR("Failed to allocate descriptor sets!")
  }
  return allocationResult;
}

void InstanceManager::WaitForFences(VkFence const *fences, uint32_t fenceCount, bool waitForAll,
                                    uint32_t timeout) const {
  VULKAN_ASSERT(vkWaitForFences(graphicsHandler, fenceCount, fences, waitForAll, timeout),
                "I have no idea how, but we just failed to wait on a fence.")
}

void InstanceManager::ResetFences(VkFence const *fences, uint32_t fenceCount) const {
    VULKAN_ASSERT(vkResetFences(graphicsHandler, fenceCount, fences), "Failed to reset fences!")}

uint32_t InstanceManager::GetNextSwapchainImageIndex(VkResult &acquisitionResult, VkSwapchainKHR const &swapchain,
                                                     VkSemaphore const &semaphore, VkFence const &fence,
                                                     uint32_t timeout) const {
  PROFILE_FUNCTION()
  uint32_t index;
  acquisitionResult = vkAcquireNextImageKHR(graphicsHandler, swapchain, timeout, semaphore, fence, &index);
  if (acquisitionResult == VK_ERROR_OUT_OF_DATE_KHR) {
    ENGINE_WARNING("Swapchain is out of date!")
  } else if (acquisitionResult == VK_SUBOPTIMAL_KHR) {
    ENGINE_WARNING("Swapchain is suboptimal!")
  } else if (acquisitionResult != VK_SUCCESS) {
    ENGINE_ERROR("Failed to acquire swapchain image!")
  }
  return index;
}

uint32_t InstanceManager::GetGraphicsFamily() const { return FindQueueFamilies(gpu, surface).graphicsFamily.value(); }

uint32_t InstanceManager::GetPresentFamily() const { return FindQueueFamilies(gpu, surface).presentFamily.value(); }

void InstanceManager::PickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    ENGINE_ERROR("Failed to find gpu with Vulkan support!")
  }
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());

  VkSurfaceKHR &surfaceRef = surface;
  auto bestDevice =
      std::max_element(devices.begin(), devices.end(), [surfaceRef](VkPhysicalDevice a, VkPhysicalDevice b) {
        return PhysicalDeviceScore(a, surfaceRef) < PhysicalDeviceScore(b, surfaceRef);
      });

  if (PhysicalDeviceScore(*bestDevice, surface) == 0) {
    ENGINE_ERROR("Device has no GPU with every necessary feature");
  }

  gpu = *bestDevice;
}

InstanceManager::InstanceManager()
    : vulkanInstance(VK_NULL_HANDLE), debugMessenger(VK_NULL_HANDLE), surface(VK_NULL_HANDLE), gpu(VK_NULL_HANDLE),
      graphicsHandler(VK_NULL_HANDLE) {}

SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR presentationSurface) {
  SwapchainSupportDetails details{};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, presentationSurface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, presentationSurface, &formatCount, nullptr);
  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, presentationSurface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, presentationSurface, &presentModeCount, nullptr);
  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, presentationSurface, &presentModeCount,
                                              details.presentModes.data());
  }

  return details;
}

} // namespace Engine::Graphics