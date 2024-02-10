#include "InstanceManager.h"

#include <vector>
#include "glfw3.h"
#include "../Macros.h"
#include "../Debug/Logging.h"
#include "../WindowManager.h"
#include <algorithm>
#include <set>

namespace Engine::Graphics
{

// Callback function for validation layers
// TODO: Write nice formatting function for pMessage
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        Debug::Logging::PrintError("Validation", pCallbackData->pMessage);
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

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, 
    VkDebugUtilsMessengerEXT* pDebugMessenger) 
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

// </Proxies for external vulkan functions>

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char *> requiredExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

//vulkan 1.3 features
VkPhysicalDeviceVulkan13Features requiredFeatures13{
    .synchronization2 = true,
    .dynamicRendering = true,
};


//vulkan 1.2 features
VkPhysicalDeviceVulkan12Features requiredFeatures12{
    .descriptorIndexing = true,
    .bufferDeviceAddress = true,
};

#define IMPL(name) (!required.name || supported.name)

bool SupportsRequiredFeatures12(VkPhysicalDeviceVulkan12Features supported, VkPhysicalDeviceVulkan12Features required) {
    return IMPL(samplerMirrorClampToEdge)
        && IMPL(drawIndirectCount)
        && IMPL(storageBuffer8BitAccess)
        && IMPL(uniformAndStorageBuffer8BitAccess)
        && IMPL(storagePushConstant8)
        && IMPL(shaderBufferInt64Atomics)
        && IMPL(shaderSharedInt64Atomics)
        && IMPL(shaderFloat16)
        && IMPL(shaderInt8)
        && IMPL(descriptorIndexing)
        && IMPL(shaderInputAttachmentArrayDynamicIndexing)
        && IMPL(shaderUniformTexelBufferArrayDynamicIndexing)
        && IMPL(shaderStorageTexelBufferArrayDynamicIndexing)
        && IMPL(shaderUniformBufferArrayNonUniformIndexing)
        && IMPL(shaderSampledImageArrayNonUniformIndexing)
        && IMPL(shaderStorageBufferArrayNonUniformIndexing)
        && IMPL(shaderStorageImageArrayNonUniformIndexing)
        && IMPL(shaderInputAttachmentArrayNonUniformIndexing)
        && IMPL(shaderUniformTexelBufferArrayNonUniformIndexing)
        && IMPL(shaderStorageTexelBufferArrayNonUniformIndexing)
        && IMPL(descriptorBindingUniformBufferUpdateAfterBind)
        && IMPL(descriptorBindingSampledImageUpdateAfterBind)
        && IMPL(descriptorBindingStorageImageUpdateAfterBind)
        && IMPL(descriptorBindingStorageBufferUpdateAfterBind)
        && IMPL(descriptorBindingUniformTexelBufferUpdateAfterBind)
        && IMPL(descriptorBindingStorageTexelBufferUpdateAfterBind)
        && IMPL(descriptorBindingUpdateUnusedWhilePending)
        && IMPL(descriptorBindingPartiallyBound)
        && IMPL(descriptorBindingVariableDescriptorCount)
        && IMPL(runtimeDescriptorArray)
        && IMPL(samplerFilterMinmax)
        && IMPL(scalarBlockLayout)
        && IMPL(imagelessFramebuffer)
        && IMPL(uniformBufferStandardLayout)
        && IMPL(shaderSubgroupExtendedTypes)
        && IMPL(separateDepthStencilLayouts)
        && IMPL(hostQueryReset)
        && IMPL(timelineSemaphore)
        && IMPL(bufferDeviceAddress)
        && IMPL(bufferDeviceAddressCaptureReplay)
        && IMPL(bufferDeviceAddressMultiDevice)
        && IMPL(vulkanMemoryModel)
        && IMPL(vulkanMemoryModelDeviceScope)
        && IMPL(vulkanMemoryModelAvailabilityVisibilityChains)
        && IMPL(shaderOutputViewportIndex)
        && IMPL(shaderOutputLayer)
        && IMPL(subgroupBroadcastDynamicId);
}

bool SupportsRequiredFeatures13(VkPhysicalDeviceVulkan13Features supported, VkPhysicalDeviceVulkan13Features required) {
    return IMPL(robustImageAccess)
        && IMPL(inlineUniformBlock)
        && IMPL(descriptorBindingInlineUniformBlockUpdateAfterBind)
        && IMPL(pipelineCreationCacheControl)
        && IMPL(privateData)
        && IMPL(shaderDemoteToHelperInvocation)
        && IMPL(shaderTerminateInvocation)
        && IMPL(subgroupSizeControl)
        && IMPL(computeFullSubgroups)
        && IMPL(synchronization2)
        && IMPL(textureCompressionASTC_HDR)
        && IMPL(shaderZeroInitializeWorkgroupMemory)
        && IMPL(dynamicRendering)
        && IMPL(shaderIntegerDotProduct)
        && IMPL(maintenance4);
}

// Only true if in debug mode and layers are available
bool enableValidationLayers = false;

std::vector<const char *> GetRequiredExtensions() 
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> required(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if(enableValidationLayers) { required.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

    return required;
}

bool CheckExtensionAvailability(std::vector<const char *> const &requiredExtensions) 
{
    uint32_t providedExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &providedExtensionCount, nullptr);
    std::vector<VkExtensionProperties> providedExtensions(providedExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &providedExtensionCount, providedExtensions.data());

    std::vector<const char *> required(requiredExtensions);

#ifndef NDEBUG
    ENGINE_MESSAGE("Supported instance extensions:")
#endif
    for(auto extension : providedExtensions) { 
#ifndef NDEBUG
        ENGINE_MESSAGE("\t{}", extension.extensionName) 
#endif
        required.erase(std::remove_if(required.begin(), required.end(), [extension](const char* str){ return strcmp(extension.extensionName, str); }), required.end());
    }

    return required.empty();
}

bool CheckValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::vector<const char *> required(validationLayers);

    for(auto layer : availableLayers) { required.erase(std::remove_if(required.begin(), required.end(), [layer](const char * str){ return strcmp(layer.layerName, str); }), required.end()); }

    return required.empty();
}

void FillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    // TODO: Allow user to determine what types of layer messages they want to see
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
}

InstanceManager::InstanceManager()
{
}

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR const & presentationSurface) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    QueueFamilyIndices indices;

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) { indices.graphicsFamily = i; }
        VkBool32 canPresent;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, presentationSurface, &canPresent);
        if (canPresent) { indices.presentFamily = i; }
        if (indices.graphicsFamily.has_value() && indices.presentFamily.has_value()) { break; }
        i++;
    }

    return indices;
}

InstanceManager::~InstanceManager()
{
    vkDestroyDevice(graphicsHandler, nullptr);
    vkDestroySurfaceKHR(vulkanInstance, surface, nullptr);
    if(enableValidationLayers) { DestroyDebugUtilsMessengerEXT(vulkanInstance, debugMessenger, nullptr); }
    vkDestroyInstance(vulkanInstance, nullptr);
}

void InstanceManager::CreateInstance(const char *applicationName)
{
    VkApplicationInfo applicationInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = applicationName,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0), // TODO: Make accessible to user
        .pEngineName = "TBNgine",
        .engineVersion = ENGINE_VERSION,                // TODO: Figure out what engine version and api version mean
        .apiVersion = VK_API_VERSION_1_3
    };

    auto requiredExtensions = GetRequiredExtensions();

    if (!CheckExtensionAvailability(requiredExtensions)) { ENGINE_ERROR("Some needed GLFW extensions are unavailable!") }

    VkInstanceCreateInfo createInfo {  
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &applicationInfo,
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data(),
    };

    if (enableValidationLayers) { 
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
        FillDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        createInfo.pNext = &debugCreateInfo;
    } else { createInfo.enabledLayerCount = 0; }

    VULKAN_ASSERT(vkCreateInstance(&createInfo, nullptr, &vulkanInstance), "Failed to create vulkan Instance!")
}

#ifndef NDEBUG
void InstanceManager::EnableValidationLayers()
{
    enableValidationLayers = true;
}

void InstanceManager::SetupDebugMessenger()
{
    if(!enableValidationLayers) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo {};

    FillDebugMessengerCreateInfo(createInfo);

    VULKAN_ASSERT(CreateDebugUtilsMessengerEXT(vulkanInstance, &createInfo, nullptr, &debugMessenger), "Failed to create debug messenger!")
}
#endif

bool DeviceHasRequiredExtensions(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> required(requiredExtensions.begin(), requiredExtensions.end());

    for (const auto& extension : availableExtensions) {
        required.erase(extension.extensionName);
    }

    return required.empty();
}

uint32_t PhysicalDeviceScore(VkPhysicalDevice device, VkSurfaceKHR const & presentationSurface) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    VkPhysicalDeviceVulkan13Features vulkan13Features { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    VkPhysicalDeviceVulkan12Features vulkan12Features { 
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &vulkan13Features
    };
    VkPhysicalDeviceVulkan11Features vulkan11Features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &vulkan12Features
    };
    VkPhysicalDeviceFeatures2 deviceFeatures { 
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &vulkan11Features 
    };
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

    bool hasNecessaryFeatures = deviceFeatures.features.geometryShader
                            &&  SupportsRequiredFeatures12(vulkan12Features, requiredFeatures12)
                            &&  SupportsRequiredFeatures13(vulkan13Features, requiredFeatures13);
    
    return  hasGraphicsFamily 
        *   hasPresentFamily
        *   extensionsSupported
        *   swapchainAdequate
        *   hasNecessaryFeatures
        *   (
                deviceProperties.limits.maxImageDimension2D 
            +   1000 * isDedicatedGPU
        );
}

void InstanceManager::CreateLogicalDevice()
{
    auto queueFamilies = FindQueueFamilies(gpu, surface);

    float queuePriority = 1.0f; // WARN: Magic number

    std::set<uint32_t> uniqueIndices = { queueFamilies.graphicsFamily.value(), queueFamilies.presentFamily.value() };
    std::vector<VkDeviceQueueCreateInfo> queueInfos {};

    for(auto family : uniqueIndices) {
        VkDeviceQueueCreateInfo queueInfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamilies.graphicsFamily.value(),
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };
        queueInfos.push_back(queueInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures {};

    VkDeviceCreateInfo deviceInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size()),
        .pQueueCreateInfos = queueInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data(),
        .pEnabledFeatures = &deviceFeatures,
    };

    if (enableValidationLayers) {
        deviceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        deviceInfo.enabledLayerCount = 0;
    }

    VULKAN_ASSERT(vkCreateDevice(gpu, &deviceInfo, nullptr, &graphicsHandler), "Failed to create logical device!")

    vkGetDeviceQueue(graphicsHandler, queueFamilies.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(graphicsHandler, queueFamilies.presentFamily.value(), 0, &presentQueue);
}

void InstanceManager::CreateSwapchain(
    VkSurfaceFormatKHR const & surfaceFormat, 
    VkPresentModeKHR const & presentMode, 
    VkExtent2D const & extent, 
    uint32_t const & imageCount, 
    VkSwapchainKHR const & oldSwapchain, 
    VkSwapchainKHR * swapchain)
{
    auto details = QuerySwapchainSupport(instance->gpu, instance->surface);
    VkSwapchainCreateInfoKHR swapchainInfo {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = instance->surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent, 
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = details.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = oldSwapchain
    };

    QueueFamilyIndices indices = FindQueueFamilies(instance->gpu, instance->surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 2;
        swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.queueFamilyIndexCount = 0; // Optional
        swapchainInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    VULKAN_ASSERT(vkCreateSwapchainKHR(instance->graphicsHandler, &swapchainInfo, nullptr, swapchain), "Failed to create swapchain!")
}

void InstanceManager::GetSwapchainImages(VkSwapchainKHR const & swapchain, std::vector<VkImage> &images)
{
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(instance->graphicsHandler, swapchain, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(instance->graphicsHandler, swapchain, &imageCount, images.data());
}

void InstanceManager::CreateImageView(VkImageViewCreateInfo const *createInfo, VkImageView *view){
    VULKAN_ASSERT(vkCreateImageView(instance->graphicsHandler, createInfo, nullptr, view), "Failed to create image view!")}

void InstanceManager::CreateCommandPool(VkCommandPoolCreateInfo const *createInfo, VkCommandPool *commandPool)
{
    VULKAN_ASSERT(vkCreateCommandPool(instance->graphicsHandler, createInfo, nullptr, commandPool), "Failed to create command pool!")
}

void InstanceManager::AllocateCommandBuffers(VkCommandBufferAllocateInfo const *allocInfo, VkCommandBuffer *commandBuffers)
{
    VULKAN_ASSERT(vkAllocateCommandBuffers(instance->graphicsHandler, allocInfo, commandBuffers), "Failed to allocate command buffers!")
}

uint32_t InstanceManager::GetGraphicsFamily()
{
    return FindQueueFamilies(instance->gpu, instance->surface).graphicsFamily.value();
}

void InstanceManager::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);
    if(deviceCount == 0) { ENGINE_ERROR("Failed to find gpu with Vulkan support!") }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());

    VkSurfaceKHR& surfaceRef = surface;
    auto bestDevice = std::max_element(devices.begin(), devices.end(), [surfaceRef](VkPhysicalDevice a, VkPhysicalDevice b) { return PhysicalDeviceScore(a, surfaceRef) < PhysicalDeviceScore(b, surfaceRef); });

    if (PhysicalDeviceScore(*bestDevice, surface) == 0) { ENGINE_ERROR("Device has no GPU with every necessary feature"); }

    gpu = *bestDevice;
}

void InstanceManager::Init(const char * applicationName)
{
    instance = new InstanceManager();
    // Only set up vulkan validation if in debug mode
#ifndef NDEBUG
    if(CheckValidationLayerSupport()) { instance->EnableValidationLayers(); }
    else { ENGINE_WARNING("Some requested validation layers could not be provided.") }
#endif
    instance->CreateInstance(applicationName);      // Instance must be created after validation layer availability has been checked
#ifndef NDEBUG
    instance->SetupDebugMessenger();
#endif
    WindowManager::GetMainWindow()->CreateSurfaceOnWindow(instance->vulkanInstance, &instance->surface);
    instance->PickPhysicalDevice();
    instance->CreateLogicalDevice();
}

void InstanceManager::Cleanup()
{

    delete instance;
}

SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR presentationSurface)
{
    SwapchainSupportDetails details {};
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
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, presentationSurface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

} // namespace Engine::Graphics