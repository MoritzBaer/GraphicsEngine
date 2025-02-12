#pragma once

#include "ImGUIManager.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#include "vulkan/vulkan.h"

#include "Debug/Profiling.h"
#include "Graphics/InstanceManager.h"
#include "Graphics/Renderer.h"
#include "Graphics/VulkanUtil.h"
#include "WindowManager.h"

#include "Core/Time.h"

#include "Editor/Publishable.h"

#include "Editor/SceneView.h"

namespace Engine::Graphics {
ImGUIManager::ImGUIManager(InstanceManager const *instanceManager)
    : instanceManager(instanceManager), views(), showImGuiDemo(false) {}

ImGUIManager::~ImGUIManager() {
  ImGui_ImplVulkan_Shutdown();
  instanceManager->DestroyDescriptorPool(imGUIPool);
}

using Editor::Publication;
using Editor::Publishable;

void ImGUIManager::InitImGUIOnWindow(Window const *window, VkFormat swapchainFormat) {
  PROFILE_FUNCTION()

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

  VkDescriptorPoolCreateInfo poolCreateInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                                            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
                                            .maxSets = 1000,
                                            .poolSizeCount = static_cast<uint32_t>(std::size(poolSizes)),
                                            .pPoolSizes = poolSizes};
  instanceManager->CreateDescriptorPool(&poolCreateInfo, &imGUIPool);

  ImGui::CreateContext();
  window->InitImGUIOnWindow();

  VkFormat colourAttachmentFormat = swapchainFormat;
  ImGui_ImplVulkan_InitInfo imGUIInfo{
      .DescriptorPool = imGUIPool,
      .MinImageCount = 3,
      .ImageCount = 3,
      .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
      .UseDynamicRendering = true,
      .PipelineRenderingCreateInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                                   .colorAttachmentCount = 1,
                                   .pColorAttachmentFormats = &colourAttachmentFormat}};

  instanceManager->FillImGUIInitInfo(imGUIInfo);

  ImGui_ImplVulkan_Init(&imGUIInfo);
  ImGui_ImplVulkan_CreateFontsTexture();

  // Enable docking for whole application
  ImGui::GetIO().ConfigFlags = ImGuiConfigFlags_DockingEnable;
}

void ImGUIManager::BeginFrame() {
  PROFILE_FUNCTION()
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  if (ImGui::Begin("Debug GUI")) {
    ImGui::Text("FPS: %.1f", 1.0f / Time::deltaTime);
    ImGui::SameLine();
    if (showImGuiDemo) {
      if (ImGui::Button("Hide demo window")) {
        showImGuiDemo = false;
      }
    } else if (ImGui::Button("Show demo window")) {
      showImGuiDemo = true;
    }
    if (showImGuiDemo) {
      ImGui::ShowDemoWindow();
    }
    ImGui::End();
  }

  for (auto view : views) {
    view->Draw();
  }

  ImGui::Render();
}

void ImGUIManager::RegisterView(ImGUIView *view) { views.push_back(view); }

} // namespace Engine::Graphics
