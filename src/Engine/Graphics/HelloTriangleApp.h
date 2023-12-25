#pragma once

#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"
#include "vulkan/vulkan.h"
#include <vector>
#include <optional>

class HelloTriangleApp
{
private:

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void Cleanup();

    void CreateVKInstance();
    bool CheckValidationLayerSupport();
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice const &device) const;
    uint32_t PhysicalDeviceScore(VkPhysicalDevice const &device) const;
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSurface();

    GLFWwindow *window;

    const uint16_t WINDOW_WIDTH = 1600;
    const uint16_t WINDOW_HEIGHT = 900;

    VkInstance vulkanInstance;
    VkPhysicalDevice physicalGPU = VK_NULL_HANDLE;
    VkDevice graphicsHandler;
    VkQueue graphicsQueue;
    VkQueue presentationQueue;
    VkSurfaceKHR surface;
    
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

public:
    void Run();
};
