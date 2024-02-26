#pragma once

#include "vulkan/vulkan.h"
#include "../Macros.h"
#include <vector>
#include <array>
#include "CommandQueue.h"

namespace Engine::Graphics
{

    class Renderer
    {
        _SINGLETON(Renderer)

        struct FrameResources {
            CommandQueue queue;
            VkSemaphore swapchainSemaphore;
            VkSemaphore renderSemaphore;
            VkFence renderFence;

            void Create();
            void Destroy();
        };

        static const uint32_t MAX_FRAME_OVERLAP = 2;

        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSwapchainKHR swapchain;
        VkFormat swapchainFormat;
        VkExtent2D swapchainExtent;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;
        std::array<FrameResources, MAX_FRAME_OVERLAP> frameResources; 
        uint32_t currentFrame = 0;

        void CreateSwapchain();
        void Draw() const;

        inline FrameResources const & CurrentResources() const { return frameResources[currentFrame % MAX_FRAME_OVERLAP]; }

    public:
        static inline void DrawFrame() { instance->Draw(); instance->currentFrame++; }     // Signature is likely to change (for example a list of render objects will have to be passed somehow)
    };
    
} // namespace Engine::Graphics


