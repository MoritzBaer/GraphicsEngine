#include "HelloTriangleApp.h"

#include <stdexcept>
#include <iostream>
#include <optional>

void HelloTriangleApp::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Triangle", nullptr, nullptr);
}

void HelloTriangleApp::InitVulkan()
{
    CreateVKInstance();
    PickPhysicalDevice();
}

void HelloTriangleApp::MainLoop()
{
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void HelloTriangleApp::Cleanup()
{
    glfwDestroyWindow(window);
    glfwTerminate();

    vkDestroyInstance(vulkanInstance, nullptr);
}
    
bool HelloTriangleApp::CheckValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char* validationLayer : validationLayers) {
        if(std::find_if(
                availableLayers.begin(), 
                availableLayers.end(), 
                [validationLayer](const VkLayerProperties &prop) { return (strcmp(validationLayer, prop.layerName)) != 0; }) 
            == availableLayers.end()) { return false; }
    }

    return true;
}

uint32_t PhysicalDeviceScore(VkPhysicalDevice const &device) { 
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    auto queueFamilies = FindQueueFamilies(device);

    return  ( 1000 * (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) )
        * deviceFeatures.geometryShader
        * queueFamilies.graphicsFamily.has_value();
}

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice const &device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    auto it = std::find_if(queueFamilies.begin(), queueFamilies.end(), [](VkQueueFamilyProperties const &family) { return family.queueFlags & VK_QUEUE_GRAPHICS_BIT; });
    if(it != queueFamilies.end()) { indices.graphicsFamily = static_cast<uint32_t>(it - queueFamilies.begin()); }

    return indices;
}

void HelloTriangleApp::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);
    if(deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());

    auto it = std::max_element(devices.begin(), devices.end(), [](VkPhysicalDevice const &a, VkPhysicalDevice const &b) { return PhysicalDeviceScore(a) < PhysicalDeviceScore(b); });
    if(it == devices.end()) { throw std::runtime_error("Failed to find a suitable GPU!"); }
    physicalGPU = *it;
}

void HelloTriangleApp::CreateVKInstance()
{
    if (enableValidationLayers && !CheckValidationLayerSupport()) { throw std::runtime_error("validation layers requested, but not available!"); }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Vulkan Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    if(enableValidationLayers) {
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else { instanceCreateInfo.enabledLayerCount = 0; }

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = nullptr;

    instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;
    instanceCreateInfo.enabledLayerCount = 0;

    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &vulkanInstance);

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensionProperties(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());

    std::cout << "available extensions:\n";

    for (const auto& extension : extensionProperties) { std::cout << '\t' << extension.extensionName << '\n'; }
}

void HelloTriangleApp::Run()
{
    InitWindow();
    InitVulkan();
    MainLoop();
    Cleanup();
}