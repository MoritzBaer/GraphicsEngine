#include "Renderer.h"
#include "glfw3.h"
#include "InstanceManager.h"
#include "../WindowManager.h"
#include <algorithm>

namespace Engine::Graphics
{
    void Renderer::Init() { 
        instance = new Renderer(); 
        instance->CreateSwapchain();
    }

    void Renderer::Cleanup() { delete instance; }

    Renderer::Renderer() {}
    Renderer::~Renderer() { InstanceManager::DestroySwapchain(swapchain); }

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
    }

} // namespace Engine::Graphics
