#include "HelloTriangleApp.h"

#include <stdexcept>
#include <iostream>
#include <set>
#include <limits>
#include <algorithm>

VkSurfaceFormatKHR HelloTriangleApp::ChooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const {
    auto it = std::find_if(availableFormats.begin(), availableFormats.end(), [](VkSurfaceFormatKHR const &format) { return 
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    &&  format.format == VK_FORMAT_B8G8R8A8_SRGB; });
    if(it != availableFormats.end()) { return *it; }
    return availableFormats[0];
}

void HelloTriangleApp::CreateSwapchain()
{
    auto details = QuerySwapchainSupport(physicalGPU);
    auto format = ChooseSwapchainFormat(details.surfaceFormats);
    auto presentMode = ChooseSwapchainPresentMode(details.presentModes);
    auto extent = ChooseSwapExtent(details.surfaceCapabilities);

    uint32_t imageCount = details.surfaceCapabilities.minImageCount + 1;
    if(details.surfaceCapabilities.maxImageCount) { imageCount = std::min(imageCount, details.surfaceCapabilities.maxImageCount); }

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = format.format;
    swapchainCreateInfo.imageColorSpace = format.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(physicalGPU);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    swapchainCreateInfo.preTransform = details.surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;

    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(graphicsHandler, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }


    vkGetSwapchainImagesKHR(graphicsHandler, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(graphicsHandler, swapchain, &imageCount, swapchainImages.data());
    
    swapchainImageFormat = format.format;
    swapchainExtent = extent;
}

VkPresentModeKHR HelloTriangleApp::ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR> & availableModes) const {
    auto it = std::find(availableModes.begin(), availableModes.end(), VK_PRESENT_MODE_MAILBOX_KHR);
    if(it != availableModes.end()) { return *it; }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangleApp::ChooseSwapExtent(VkSurfaceCapabilitiesKHR const & capabilities) const {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

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
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
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

    vkDestroySwapchainKHR(graphicsHandler, swapchain, nullptr);
    vkDestroyDevice(graphicsHandler, nullptr);
    vkDestroySurfaceKHR(vulkanInstance, surface, nullptr);
    vkDestroyInstance(vulkanInstance, nullptr);
}
    
bool HelloTriangleApp::CheckValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char* validationLayer : requiredValidationLayers) {
        if(std::find_if(
                availableLayers.begin(), 
                availableLayers.end(), 
                [validationLayer](const VkLayerProperties &prop) { return (strcmp(validationLayer, prop.layerName)) != 0; }) 
            == availableLayers.end()) { return false; }
    }

    return true;
}

HelloTriangleApp::QueueFamilyIndices HelloTriangleApp::FindQueueFamilies(VkPhysicalDevice const &device) const {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for(int i = 0; i < queueFamilies.size(); i++) {
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { indices.graphicsFamily = i; }

        VkBool32 presentationSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);
        if(presentationSupport) { indices.presentFamily = i; }
    }


    return indices;
}

uint32_t HelloTriangleApp::PhysicalDeviceScore(VkPhysicalDevice const &device) const { 
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    auto queueFamilies = FindQueueFamilies(device);
    auto swapchainDetails = QuerySwapchainSupport(device);

    bool deviceHasNecessaryCapabilities = deviceFeatures.geometryShader
        && queueFamilies.graphicsFamily.has_value()
        && queueFamilies.presentFamily.has_value()
        && CheckDeviceExtensionSupport(device)
        && !swapchainDetails.surfaceFormats.empty()
        && !swapchainDetails.presentModes.empty();

    uint32_t score = ( 1000 * (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) + 1 )
        * deviceHasNecessaryCapabilities;

    return score;
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

    auto it = std::max_element(devices.begin(), devices.end(), [this](VkPhysicalDevice const &a, VkPhysicalDevice const &b) { return PhysicalDeviceScore(a) < PhysicalDeviceScore(b); });
    if(it == devices.end() || PhysicalDeviceScore(*it) == 0) { throw std::runtime_error("Failed to find a suitable GPU!"); }
    physicalGPU = *it;

#ifdef NDEBUG
#else
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalGPU, &deviceProperties);
    std::cout << "Device '" << deviceProperties.deviceName << "' selected for executing graphics commands\n";
#endif
}

void HelloTriangleApp::CreateLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(physicalGPU);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures features{};

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pEnabledFeatures = &features;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();

    // Only here for compatibility with older Vulkan implementations
    if (enableValidationLayers) {
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = requiredValidationLayers.data();
    } else {
        deviceCreateInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalGPU, &deviceCreateInfo, nullptr, &graphicsHandler) != VK_SUCCESS) { throw std::runtime_error("failed to create logical device!"); }

    vkGetDeviceQueue(graphicsHandler, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(graphicsHandler, indices.presentFamily.value(), 0, &presentationQueue);
}

void HelloTriangleApp::CreateSurface()
{
    if (glfwCreateWindowSurface(vulkanInstance, window, nullptr, &surface) != VK_SUCCESS) { throw std::runtime_error("failed to create window surface!"); }
}

bool HelloTriangleApp::CheckDeviceExtensionSupport(VkPhysicalDevice const &device) const
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());
    for(auto e : availableExtensions) { requiredExtensions.erase(e.extensionName); }
    return requiredExtensions.empty();
}

HelloTriangleApp::SwapchainSupportDetails HelloTriangleApp::QuerySwapchainSupport(VkPhysicalDevice const &device) const
{
    SwapchainSupportDetails details{};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.surfaceCapabilities);
    
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    details.surfaceFormats.resize(formatCount);
    if(formatCount) { vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.surfaceFormats.data()); }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    details.presentModes.resize(presentModeCount);
    if(formatCount) { vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &formatCount, details.presentModes.data()); }

    return details;
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
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = requiredValidationLayers.data();
    } else { instanceCreateInfo.enabledLayerCount = 0; }

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

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