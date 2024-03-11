#include "Renderer.h"
#include "GLFW/glfw3.h"
#include "InstanceManager.h"
#include "WindowManager.h"
#include <algorithm>
#include "Debug/Logging.h"
#include "VulkanUtil.h"
#include "Image.h"
#include "AssetManager.h"
#include "Graphics/ImGUIManager.h"
#include "ComputeEffect.h"
#include <vector>

namespace Engine::Graphics
{   
    struct ComputePushConstants {
        Maths::Vector4 data1, data2, data3, data4;
    };

    std::vector<ComputeEffect<ComputePushConstants>> backgroundEffects = {
        ComputeEffect<ComputePushConstants> { .name = "gradient_colour", .constants { .data1 = { 1, 0, 0, 1 }, .data2 = { 0, 0, 1, 1 } } },
        ComputeEffect<ComputePushConstants> { .name = "sky", .constants { .data1 = { 0.02f, 0, 0.05f, 0.99f } } },
    };
    uint8_t currentBackgroundEffect = 0;

    namespace vkinit
    {
        inline VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = VK_FENCE_CREATE_SIGNALED_BIT) { 
            return {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .flags = flags
            };
        }

        inline VkSubmitInfo2 SubmitInfo(
            std::vector<VkSemaphoreSubmitInfo> const & semaphoreWaitInfos,
            std::vector<VkCommandBufferSubmitInfo> const & commandBufferInfos,
            std::vector<VkSemaphoreSubmitInfo> const & semaphoreSignalInfos
        ) {
            return {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .waitSemaphoreInfoCount = static_cast<uint32_t>(semaphoreWaitInfos.size()),
                .pWaitSemaphoreInfos = semaphoreWaitInfos.data(),
                .commandBufferInfoCount = static_cast<uint32_t>(commandBufferInfos.size()),
                .pCommandBufferInfos = commandBufferInfos.data(),
                .signalSemaphoreInfoCount = static_cast<uint32_t>(semaphoreSignalInfos.size()),
                .pSignalSemaphoreInfos = semaphoreSignalInfos.data()
            };
        }
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
        mainDeletionQueue.Push(&instance->immediateResources);
        instance->InitDescriptors();
        instance->InitPipelines();
    }

    void Renderer::Cleanup() { delete instance; }

    Renderer::Renderer() {}
    Renderer::~Renderer() {
        for(auto effect : backgroundEffects) { InstanceManager::DestroyPipeline(effect.pipeline); }
        InstanceManager::DestroyPipelineLayout(backgroundEffects[0].pipelineLayout);
        descriptorAllocator.ClearDescriptors();
        descriptorAllocator.DestroyPool();
        InstanceManager::DestroyDescriptorSetLayout(renderBufferDescriptorLayout);
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

    VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, Maths::Dimension2 canvasSize) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
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
        VkExtent2D extent = ChooseSwapchainExtent(details.capabilities, windowDimension);

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

        auto computeRun = ExecuteComputePipelineCommand<ComputePushConstants>(
                backgroundEffects[currentBackgroundEffect],
                VK_PIPELINE_BIND_POINT_COMPUTE, 
                renderBufferDescriptors,
                std::ceil<uint32_t>(windowDimension[X] / 16u),
                std::ceil<uint32_t>(windowDimension[Y] / 16u),
                1
            );
        auto transitionBufferToTransferSrc = vkutil::TransitionImageCommand(renderBuffer.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        auto transitionPresenterToTransferDst = vkutil::TransitionImageCommand(swapchainImages[resourceIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        auto copyBufferToPresenter = vkutil::CopyFullImage(renderBuffer.image, swapchainImages[resourceIndex], renderBuffer.imageExtent, { swapchainExtent.width, swapchainExtent.height, 1} );
        auto transitionPresenterToPresent = vkutil::TransitionImageCommand(swapchainImages[resourceIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        auto renderImGUI = ImGUIManager::DrawFrameCommand(swapchainImageViews[resourceIndex], swapchainExtent);
        auto transitionPresenterToColourAttachment = vkutil::TransitionImageCommand(swapchainImages[resourceIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        auto commandBufferSubmitInfo = frameResources[resourceIndex].commandQueue.EnqueueCommandSequence({
            &transitionBufferToWriteable,
            &computeRun,
            &transitionPresenterToTransferDst,
            &transitionBufferToTransferSrc,
            &copyBufferToPresenter,
            &transitionPresenterToColourAttachment,
            &renderImGUI,
            &transitionPresenterToPresent
        });

        auto semaphoreWaitInfo = vkinit::SemaphoreSubmitInfo(frameResources[resourceIndex].swapchainSemaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
        auto semaphoreSignalInfo = vkinit::SemaphoreSubmitInfo(frameResources[resourceIndex].renderSemaphore, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);

        std::vector<VkCommandBufferSubmitInfo> buffer { commandBufferSubmitInfo };
        std::vector<VkSemaphoreSubmitInfo> waitSemaphores { semaphoreWaitInfo };
        std::vector<VkSemaphoreSubmitInfo> signalSemaphores { semaphoreSignalInfo };

        VkSubmitInfo2 submitInfo = vkinit::SubmitInfo(waitSemaphores, buffer, signalSemaphores);

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

    void Renderer::InitDescriptors()
    {
        std::vector<DescriptorAllocator::PoolSizeRatio> ratios { { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 } };

        descriptorAllocator.InitPool(10, ratios);

        DescriptorLayoutBuilder builder {};
        builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        renderBufferDescriptorLayout = builder.Build(VK_SHADER_STAGE_COMPUTE_BIT);
        renderBufferDescriptors = descriptorAllocator.Allocate(renderBufferDescriptorLayout);

        VkDescriptorImageInfo imageInfo {
            .imageView = renderBuffer.imageView,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL
        };

        VkWriteDescriptorSet write {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = renderBufferDescriptors,
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .pImageInfo = &imageInfo
        };

        InstanceManager::UpdateDescriptorSets(write);
    }

    void Renderer::InitPipelines()
    {
        InitBackgroundPipeline();
    }

    void Renderer::InitBackgroundPipeline()
    {
        VkPushConstantRange pushConstants {
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .offset = 0,
            .size = sizeof(ComputePushConstants)
        };

        VkPipelineLayoutCreateInfo computeLayoutInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &renderBufferDescriptorLayout,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &pushConstants
        };

        VkPipelineLayout computePipelineLayout;
        InstanceManager::CreatePipelineLayout(&computeLayoutInfo, &computePipelineLayout);
        
        VkComputePipelineCreateInfo pipelineInfo {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .layout = computePipelineLayout
        };

        for (auto & effect : backgroundEffects) {
            Shader shader = AssetManager::LoadShader(effect.name + ".comp", ShaderType::COMPUTE);
            pipelineInfo.stage = shader.GetStageInfo();

            effect.pipelineLayout = computePipelineLayout;
            InstanceManager::CreateComputePipeline(pipelineInfo, &effect.pipeline);

            shader.Destroy();
        }
    }

    void Renderer::ImmediateSubmit(Command *command)
    {
        InstanceManager::ResetFences(&instance->immediateResources.fence);
        auto commandInfo = instance->immediateResources.commandQueue.EnqueueCommandSequence({ command });
        std::vector<VkCommandBufferSubmitInfo> buffer { commandInfo };
        VkSubmitInfo2 submitInfo = vkinit::SubmitInfo({ }, buffer, { });

        VULKAN_ASSERT(vkQueueSubmit2(instance->graphicsQueue, 1, &submitInfo, instance->immediateResources.fence), "Failed to submit immediate queue")

        InstanceManager::WaitForFences(&instance->immediateResources.fence);
    }

    void Renderer::GetImGUISection()
    {
        if(ImGui::Begin("Background effects")){
            if (ImGui::BeginCombo("Choose an effect", backgroundEffects[currentBackgroundEffect].name.c_str())) 
            {
                for (int i = 0; i < backgroundEffects.size(); i++) {
                    bool isSelected = currentBackgroundEffect == i;
                    if (ImGui::Selectable(backgroundEffects[i].name.c_str(), isSelected)) { currentBackgroundEffect = i; }
                    if (isSelected) { ImGui::SetItemDefaultFocus(); }
                }

                ImGui::EndCombo();
            }
            ImGui::InputFloat4("data1", (float*)&backgroundEffects[currentBackgroundEffect].constants.data1);
            ImGui::InputFloat4("data2", (float*)&backgroundEffects[currentBackgroundEffect].constants.data2);
            ImGui::InputFloat4("data3", (float*)&backgroundEffects[currentBackgroundEffect].constants.data3);
            ImGui::InputFloat4("data4", (float*)&backgroundEffects[currentBackgroundEffect].constants.data4);

            ImGui::End();
        }
    }

    void Renderer::FrameResources::Create()
    {
        VkFenceCreateInfo fenceInfo = vkinit::FenceCreateInfo();

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

    void Renderer::ImmediateSubmitResources::Create()
    {
        auto fenceInfo = vkinit::FenceCreateInfo();
        InstanceManager::CreateFence(&fenceInfo, &fence);
        commandQueue.Create();
    }

    void Renderer::ImmediateSubmitResources::Destroy()
    {
        commandQueue.Destroy();
        InstanceManager::DestroyFence(fence);
    }

} // namespace Engine::Graphics
