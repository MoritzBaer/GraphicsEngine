#pragma once

#include "ImGUIManager.h"

#include "vulkan/vulkan.h"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

#include "WindowManager.h"
#include "Graphics/InstanceManager.h"
#include "Graphics/Renderer.h"

namespace Engine::Graphics
{
    ImGUIManager::ImGUIManager() {}
    ImGUIManager::~ImGUIManager() { InstanceManager::DestroyDescriptorPool(instance->imGUIPool); }

    void ImGUIManager::Init(Window const *window)
    {
        instance = new ImGUIManager();

        VkDescriptorPoolSize poolSizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

        VkDescriptorPoolCreateInfo poolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = 1000,
            .poolSizeCount = static_cast<uint32_t>(std::size(poolSizes)),
            .pPoolSizes = poolSizes};
        InstanceManager::CreateDescriptorPool(&poolCreateInfo, &instance->imGUIPool);

        ImGui::CreateContext();
        window->InitImGUIOnWindow();

        VkFormat colourAttachmentFormat = Renderer::GetSwapchainFormat();
        ImGui_ImplVulkan_InitInfo imGUIInfo{
            .DescriptorPool = instance->imGUIPool,
            .MinImageCount = 3,
            .ImageCount = 3,
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
            .UseDynamicRendering = true,
            .PipelineRenderingCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &colourAttachmentFormat}};

        InstanceManager::FillImGUIInitInfo(imGUIInfo);

        ImGui_ImplVulkan_Init(&imGUIInfo);
        ImGui_ImplVulkan_CreateFontsTexture();
    }

    void ImGUIManager::Cleanup()
    {
        ImGui_ImplVulkan_Shutdown();
        delete instance;
    }
    void ImGUIManager::BeginFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        Renderer::GetImGUISection();

        ImGui::Render();
    }

    void ImGUIManager::ImGUIDrawCommand::QueueExecution(VkCommandBuffer const &queue) const
    {
        VkRenderingAttachmentInfo colourAttachment{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = targetView,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
        };

        VkRenderingInfo renderInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea{
                .offset = {0, 0},
                .extent = targetExtent},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colourAttachment
        };

        vkCmdBeginRendering(queue, &renderInfo);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), queue);

        vkCmdEndRendering(queue);
    }

} // namespace Engine::Util
