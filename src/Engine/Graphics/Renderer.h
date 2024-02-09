#pragma once

#include "vulkan/vulkan.h"
#include "../Macros.h"
#include <vector>

namespace Engine::Graphics
{

    class Renderer
    {
        _SINGLETON(Renderer)

        VkSwapchainKHR swapchain;
        VkFormat swapchainFormat;
        VkExtent2D swapchainExtent;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;

        void CreateSwapchain();
    };
    
} // namespace Engine::Graphics


