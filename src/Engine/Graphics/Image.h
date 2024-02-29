#pragma once

#include "vulkan/vulkan.h"
#include "MemoryAllocator.h"
#include "InstanceManager.h"

namespace Engine::Graphics
{
    class Renderer;

    template <uint8_t Dimension>
    class Image : public Destroyable
    {
    private:
        VkImage image;
        VkImageView imageView;
        VmaAllocation allocation;
        VkExtent3D imageExtent;
        VkFormat imageFormat;

        static const VkImageType IMAGE_TYPE;
        static const VkImageViewType VIEW_TYPE;

        friend class Renderer;
    public:
        void Create(
            VkFormat format,
            VkExtent3D extent,
            VkImageUsageFlags usage, 
            VkImageAspectFlags aspectMask, 
            uint32_t mipLevels = 1, 
            uint32_t arrayLayers = 1, 
            VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT);
        void Destroy();
    };

    template <> const VkImageType Engine::Graphics::Image<1>::IMAGE_TYPE = VK_IMAGE_TYPE_1D;
    template <> const VkImageType Engine::Graphics::Image<2>::IMAGE_TYPE = VK_IMAGE_TYPE_2D;
    template <> const VkImageType Engine::Graphics::Image<3>::IMAGE_TYPE = VK_IMAGE_TYPE_3D;

    template <> const VkImageViewType Engine::Graphics::Image<1>::VIEW_TYPE = VK_IMAGE_VIEW_TYPE_1D;
    template <> const VkImageViewType Engine::Graphics::Image<2>::VIEW_TYPE = VK_IMAGE_VIEW_TYPE_2D;
    template <> const VkImageViewType Engine::Graphics::Image<3>::VIEW_TYPE = VK_IMAGE_VIEW_TYPE_3D;
    
    template<uint8_t Dimension>
    void Engine::Graphics::Image<Dimension>::Create(
        VkFormat format,
        VkExtent3D extent,
        VkImageUsageFlags usage, 
        VkImageAspectFlags aspectMask, 
        uint32_t mipLevels, 
        uint32_t arrayLayers, 
        VkSampleCountFlagBits msaaSamples
        ) {
        imageFormat = format;
        imageExtent = extent;
        
        VkImageCreateInfo imageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = IMAGE_TYPE,
            .format = imageFormat,
            .extent = imageExtent,
            .mipLevels = mipLevels,
            .arrayLayers = arrayLayers,
            .samples = msaaSamples,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = usage
        };

        mainAllocator.CreateImage(&imageCreateInfo, &image, &allocation);

        VkImageViewCreateInfo imageViewCreateInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image,
            .viewType = VIEW_TYPE,
            .format = imageFormat,
            .subresourceRange = {
                .aspectMask = aspectMask,
                .baseMipLevel = 0,
                .levelCount = mipLevels,
                .baseArrayLayer = 0,
                .layerCount = arrayLayers
            }
        };

        InstanceManager::CreateImageView(&imageViewCreateInfo, &imageView);
    }

    template <uint8_t Dimension>
    inline void Image<Dimension>::Destroy()
    {
        InstanceManager::DestroyImageView(imageView);
        mainAllocator.DestroyImage(image, allocation);
    }
} // namespace Engine::Graphics
