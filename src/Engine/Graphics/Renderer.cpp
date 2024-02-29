#include "Renderer.h"
#include "glfw3.h"
#include "InstanceManager.h"
#include "../WindowManager.h"
#include <algorithm>
#include "../Debug/Logging.h"
#include "VulkanUtil.h"
#include "Image.h"

namespace Engine::Graphics
{   
    namespace vkinit
    {
    } // namespace vkinit
    
    namespace vkutil
    {
        inline PipelineBarrierCommand TransitionImageCommand(VkImage image, VkImageLayout currentLayout, VkImageLayout targetLayout) {
            return PipelineBarrierCommand({ vkinit::ImageMemoryBarrier(image, currentLayout, targetLayout) });
        }

        inline BlitImageCommand CopyFullImage(VkImage source, VkImage destination, VkExtent3D srcExtent, VkExtent3D dstExtent) {
            VkImageBlit2 blitRegion {
                .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
                .srcSubresource = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .srcOffsets = {
                    { 0, 0, 0 },
                    { 
                        static_cast<int32_t>(srcExtent.width), 
                        static_cast<int32_t>(srcExtent.height), 
                        static_cast<int32_t>(srcExtent.depth) 
                    },
                },
                .dstSubresource = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .dstOffsets = {
                    { 0, 0, 0 },
                    { 
                        static_cast<int32_t>(dstExtent.width), 
                        static_cast<int32_t>(dstExtent.height), 
                        static_cast<int32_t>(dstExtent.depth) 
                    },
                }
            };

            return BlitImageCommand(source, destination, { blitRegion });
        }
    } // namespace vkutil
    
    void Renderer::Init(Maths::Dimension2 windowSize) { 
        instance = new Renderer(); 
        InstanceManager::GetGraphicsQueue(&instance->graphicsQueue);
        InstanceManager::GetPresentQueue(&instance->presentQueue);
        instance->windowDimension = windowSize;
        instance->CreateSwapchain();
        for(int i = 0; i < MAX_FRAME_OVERLAP; i++) { mainDeletionQueue.Push(&instance->frameResources[i]); }
    }

    void Renderer::Cleanup() { delete instance; }

    Renderer::Renderer() {}
    Renderer::~Renderer() {
        for(auto view: swapchainImageViews) { InstanceManager::DestroyImageView(view); }
        InstanceManager::DestroySwapchain(swapchain);
    }

    VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }   

    VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            Maths::Dimension2 canvasSize = WindowManager::GetMainWindow()->GetCanvasSize();

            VkExtent2D actualExtent = {
                std::clamp(canvasSize[X], capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
                std::clamp(canvasSize[Y], capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
            };

            return actualExtent;
        }
    }

    void Renderer::CreateSwapchain()
    {
        renderBufferInitialized = false;

        SwapchainSupportDetails details = InstanceManager::GetSwapchainSupport();

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapchainSurfaceFormat(details.formats);
        VkPresentModeKHR presentMode = ChooseSwapchainPresentMode(details.presentModes);
        VkExtent2D extent = ChooseSwapchainExtent(details.capabilities);

        uint32_t imageCount = std::min(details.capabilities.minImageCount + 1, details.capabilities.maxImageCount);

        InstanceManager::CreateSwapchain(
            surfaceFormat,
            presentMode,
            extent,
            imageCount,
            VK_NULL_HANDLE,
            &swapchain
        );

        swapchainExtent = extent;
        swapchainFormat = surfaceFormat.format;

        InstanceManager::GetSwapchainImages(swapchain, swapchainImages);

        // Create image views
        swapchainImageViews.resize(swapchainImages.size());
        for(int i = 0; i < swapchainImageViews.size(); i++) {
            VkImageViewCreateInfo imageViewInfo {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = swapchainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = swapchainFormat,
                .components {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            InstanceManager::CreateImageView(&imageViewInfo, swapchainImageViews.data() + i);
        }

        RecreateRenderBuffer();
        mainDeletionQueue.Push(&renderBuffer);
    }

    void Renderer::Draw() const
    {
        uint32_t resourceIndex = currentFrame % MAX_FRAME_OVERLAP;
        InstanceManager::WaitForFences(&frameResources[resourceIndex].renderFence);
        InstanceManager::ResetFences(&frameResources[resourceIndex].renderFence);
    	
        uint32_t swapchainImageIndex = InstanceManager::GetNextSwapchainImageIndex(swapchain, frameResources[resourceIndex].swapchainSemaphore);

        auto transitionBufferToWriteable = vkutil::TransitionImageCommand(renderBuffer.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        auto clearBufferColour = ClearColourCommand(
                renderBuffer.image, 
                VK_IMAGE_LAYOUT_GENERAL,
                { { 0.0f, 0.0f, abs(sin(currentFrame / 120.0f)), 1.0f } },
                { vkinit::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT) }
            );
        auto transitionBufferToTransferSrc = vkutil::TransitionImageCommand(renderBuffer.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        auto transitionPresenterToTransferDst = vkutil::TransitionImageCommand(swapchainImages[resourceIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        auto copyBufferToPresenter = vkutil::CopyFullImage(renderBuffer.image, swapchainImages[resourceIndex], renderBuffer.imageExtent, { swapchainExtent.width, swapchainExtent.height, 1} );
        auto transitionPresenterToPresent = vkutil::TransitionImageCommand(swapchainImages[resourceIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        auto commandBufferSubmitInfo = frameResources[resourceIndex].commandQueue.EnqueueCommandSequence({
            &transitionBufferToWriteable,
            &clearBufferColour,
            &transitionPresenterToTransferDst,
            &transitionBufferToTransferSrc,
            &copyBufferToPresenter,
            &transitionPresenterToPresent
        });

        auto semaphoreWaitInfo = vkinit::SemaphoreSubmitInfo(frameResources[resourceIndex].swapchainSemaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
        auto semaphoreSignalInfo = vkinit::SemaphoreSubmitInfo(frameResources[resourceIndex].renderSemaphore, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);

        VkSubmitInfo2 submitInfo {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount = 1,
            .pWaitSemaphoreInfos = &semaphoreWaitInfo,
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &commandBufferSubmitInfo,
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &semaphoreSignalInfo,
        };

        VULKAN_ASSERT(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, frameResources[resourceIndex].renderFence), "Failed to submit queue")

        VkPresentInfoKHR presentInfo {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frameResources[resourceIndex].renderSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &swapchainImageIndex
        };

        VULKAN_ASSERT(vkQueuePresentKHR(presentQueue, &presentInfo), "Failed to present image!")
    }

    void Renderer::RecreateRenderBuffer()
    {
        VkExtent3D renderBufferExtent = { windowDimension.x(), windowDimension.y(), 1 };

        if (renderBufferInitialized) { renderBuffer.Destroy(); }
        renderBuffer.Create(
            VK_FORMAT_R16G16B16A16_SFLOAT,
            renderBufferExtent,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT
        );
    }

    void Renderer::FrameResources::Create()
    {
        VkFenceCreateInfo fenceInfo { 
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT 
        };

        VkSemaphoreCreateInfo semaphoreInfo { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

        InstanceManager::CreateFence(&fenceInfo, &renderFence);

        InstanceManager::CreateSemaphore(&semaphoreInfo, &renderSemaphore);
        InstanceManager::CreateSemaphore(&semaphoreInfo, &swapchainSemaphore);

        commandQueue.Create();
        deletionQueue.Create();
    }

    void Renderer::FrameResources::Destroy()
    {
        deletionQueue.Destroy();
        commandQueue.Destroy();
        InstanceManager::DestroySemaphore(swapchainSemaphore);
        InstanceManager::DestroySemaphore(renderSemaphore);
        InstanceManager::DestroyFence(renderFence);
    }

} // namespace Engine::Graphics
