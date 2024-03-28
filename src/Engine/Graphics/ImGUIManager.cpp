#pragma once

#include "ImGUIManager.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#include "vulkan/vulkan.h"

#include "Debug/Profiling.h"
#include "Graphics/InstanceManager.h"
#include "Graphics/Renderer.h"
#include "VulkanUtil.h"
#include "WindowManager.h"

#include "Core/Time.h"

#include "Editor/Publishable.h"

#include "Editor/SceneView.h"

namespace Engine::Graphics {
ImGUIManager::ImGUIManager() {}
ImGUIManager::~ImGUIManager() { InstanceManager::DestroyDescriptorPool(instance->imGUIPool); }

using Editor::Publication;
using Editor::Publishable;

struct TestPublishable : public Publishable {
  Maths::Vector3 position, rotation, scale;
  Maths::Vector4 colour;

  TestPublishable(Maths::Vector3 pos, Maths::Vector3 rot, Maths::Vector3 scl)
      : position(pos), rotation(rot), scale(scl), Publishable("TestPublishable") {}
  TestPublishable() : position(), rotation(), scale(), Publishable("TestPublishable") {}

  std::vector<Publication> GetPublications() {
    return {PUBLISH(position), PUBLISH(rotation), PUBLISH_SLIDER(scale, 0.01f, 1000.0f, 1.0f), PUBLISH(colour)};
  }
} testPublishable;
void ImGUIManager::Init(Window const *window) {
  PROFILE_FUNCTION()
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

  VkDescriptorPoolCreateInfo poolCreateInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
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
      .PipelineRenderingCreateInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                                   .colorAttachmentCount = 1,
                                   .pColorAttachmentFormats = &colourAttachmentFormat}};

  InstanceManager::FillImGUIInitInfo(imGUIInfo);

  ImGui_ImplVulkan_Init(&imGUIInfo);
  ImGui_ImplVulkan_CreateFontsTexture();

  // Enable docking for whole application
  ImGui::GetIO().ConfigFlags = ImGuiConfigFlags_DockingEnable;

  testPublishable = TestPublishable(Maths::Vector3{10, -2, 4}, Maths::Vector3{0, 90, 0}, Maths::Vector3{0.3, 0.2, 0.5});
}

void ImGUIManager::Cleanup() {
  ImGui_ImplVulkan_Shutdown();
  delete instance;
}

void ImGUIManager::BeginFrame() {
  PROFILE_FUNCTION()
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  if (ImGui::Begin("Debug GUI")) {
    ImGui::Text("FPS: %.1f", 1.0f / Time::deltaTime);
    Renderer::GetImGUISection();
    ImGui::End();
  }

  for (auto view : instance->views) {
    view->Draw();
  }

  ImGui::Render();
}

void ImGUIManager::RegisterView(ImGUIView const *view) { instance->views.push_back(view); }

void ImGUIManager::ImGUIDrawCommand::QueueExecution(VkCommandBuffer const &queue) const {
  VkRenderingAttachmentInfo colourAttachment = vkinit::ColourAttachmentInfo(targetView);
  VkRenderingInfo renderInfo = vkinit::RenderingInfo(colourAttachment, targetExtent);

  vkCmdBeginRendering(queue, &renderInfo);

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), queue);

  vkCmdEndRendering(queue);
}

} // namespace Engine::Graphics
