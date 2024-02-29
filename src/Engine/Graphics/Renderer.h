#pragma once

#include "vulkan/vulkan.h"
#include "../Macros.h"
#include <vector>
#include <array>
#include "CommandQueue.h"
#include "../DeletionQueue.h"
#include "../Maths/Dimension.h"
#include "Image.h"

namespace Engine::Graphics
{

    class Renderer
    {
        _SINGLETON(Renderer, Maths::Dimension2 windowSize)

        struct FrameResources : public Initializable {
            CommandQueue commandQueue;
            DeletionQueue deletionQueue;
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

        Maths::Dimension2 windowDimension;
        bool renderBufferInitialized;
        Image<2> renderBuffer;

        void CreateSwapchain();
        void Draw() const;

        void RecreateRenderBuffer();

        inline FrameResources const & CurrentResources() const { return frameResources[currentFrame % MAX_FRAME_OVERLAP]; }

    public:
        // Signature is likely to change (for example a list of render objects will have to be passed somehow)
        static inline void DrawFrame() { 
            instance->Draw(); 
            instance->frameResources[instance->currentFrame % MAX_FRAME_OVERLAP].deletionQueue.Flush();
            instance->currentFrame++; 
        }     

        static inline void SetWindowSize(Maths::Dimension2 newSize) { 
            instance->windowDimension = newSize;
            instance->RecreateRenderBuffer();
        }
    };
    
} // namespace Engine::Graphics


