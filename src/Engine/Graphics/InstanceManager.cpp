#include "InstanceManager.h"

#include <vector>
#include "glfw3.h"
#include "../Macros.h"
#include "../Debug/Logging.h"
#include <algorithm>

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

InstanceManager::InstanceManager(const char * applicationName)
{
    // Only set up vulkan validation if in debug mode
#ifndef NDEBUG
    if(CheckValidationLayerSupport()) { EnableValidationLayers(); }
    else { ENGINE_WARNING("Some requested validation layers could not be provided.") }
#endif
    CreateInstance(applicationName);    // Instance must be created after validation layer availability has been checked
#ifndef NDEBUG
    SetupDebugMessenger();
#endif
}

InstanceManager::~InstanceManager()
{
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
        .apiVersion = VK_API_VERSION_1_0
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

void InstanceManager::Init(const char * applicationName)
{
    instanceManagerInstance = new InstanceManager(applicationName);
}

void InstanceManager::Cleanup()
{
    delete instanceManagerInstance;
}
    
} // namespace Engine::Graphics