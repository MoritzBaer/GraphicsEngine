#pragma once

#include "vulkan/vulkan.h"
#include "../Macros.h"
#include <vector>
#include <array>

namespace Engine::Graphics
{

    class Renderer
    {
        _SINGLETON(Renderer)

        struct FrameResources {
            VkCommandPool commandPool;
            VkCommandBuffer mainCommandBuffer;
        };

        static const uint32_t MAX_FRAME_OVERLAP = 2;

        VkSwapchainKHR swapchain;
        VkFormat swapchainFormat;
        VkExtent2D swapchainExtent;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;
        std::array<FrameResources, MAX_FRAME_OVERLAP> frameResources; 
        uint32_t currentFrame = 0;

        void CreateSwapchain();
        void CreateFrameResources();

        inline FrameResources const & CurrentResources() const { return frameResources[currentFrame % MAX_FRAME_OVERLAP]; }
    };
    
} // namespace Engine::Graphics


