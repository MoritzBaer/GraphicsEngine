#pragma once

#include "glfw3.h"
#include "vulkan/vulkan.h"
#include <vector>

class HelloTriangleApp
{
private:
    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void Cleanup();

    void CreateVKInstance();

    bool CheckValidationLayerSupport();
    void PickPhysicalDevice();
    void CreateLogicalDevice();

    GLFWwindow *window;

    const uint16_t WINDOW_WIDTH = 1600;
    const uint16_t WINDOW_HEIGHT = 900;

    VkInstance vulkanInstance;
    VkPhysicalDevice physicalGPU = VK_NULL_HANDLE;
    VkDevice graphicsHandler;
    VkQueue graphicsQueue;
    
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
