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

#include "Publishable.h"

namespace Engine::Graphics {
ImGUIManager::ImGUIManager() {}
ImGUIManager::~ImGUIManager() { InstanceManager::DestroyDescriptorPool(instance->imGUIPool); }

struct TestPublishable : public Publishable {
  Maths::Vector3 position, rotation, scale;
  Maths::Vector4 colour;

  TestPublishable(Maths::Vector3 pos, Maths::Vector3 rot, Maths::Vector3 scl)
      : position(pos), rotation(rot), scale(scl) {}
  TestPublishable() : position(), rotation(), scale() {}

  std::vector<Publication> GetPublications() {
    return {Publication{.label = "Position",
                        .type = Publication::Type::FLOAT3,
                        .style = Publication::Style::DRAG,
                        .referencedPointer = &position},
            Publication{.label = "Rotation",
                        .type = Publication::Type::FLOAT3,
                        .style = Publication::Style::DRAG,
                        .referencedPointer = &rotation},
            Publication{.label = "Scale",
                        .type = Publication::Type::FLOAT3,
                        .style = Publication::Style::SLIDER,
                        .flags = Publication::Flags::RANGE,
                        .floatRange{.min = 0.01f, .max = 1000.0f, .step = 1.0f},
                        .referencedPointer = &scale},
            Publication{.label = "Colour", .type = Publication::Type::COLOUR_PICKER, .referencedPointer = &colour}};
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

#define DRAW_DRAG_INT(func, ranged)                                                                                    \
  if (ranged) {                                                                                                        \
    func(publication.label, (int *)publication.referencedPointer, publication.intRange.min, publication.intRange.max,  \
         publication.intRange.step);                                                                                   \
  } else {                                                                                                             \
    func(publication.label, (int *)publication.referencedPointer);                                                     \
  }

#define DRAW_DRAG_FLOAT(func, ranged)                                                                                  \
  if (ranged) {                                                                                                        \
    func(publication.label, (float *)publication.referencedPointer, publication.floatRange.min,                        \
         publication.floatRange.max, publication.floatRange.step);                                                     \
  } else {                                                                                                             \
    func(publication.label, (float *)publication.referencedPointer);                                                   \
  }

#define DRAW_SLIDER_INT(func, ranged)                                                                                  \
  func(publication.label, (int *)publication.referencedPointer, publication.intRange.min, publication.intRange.max)

#define DRAW_SLIDER_FLOAT(func, ranged)                                                                                \
  if (ranged) {                                                                                                        \
    func(publication.label, (float *)publication.referencedPointer, publication.floatRange.min,                        \
         publication.floatRange.max);                                                                                  \
  } else {                                                                                                             \
    ENGINE_ERROR("Publication labelled {} was styled as slider but the RANGE flag has not been set!",                  \
                 publication.label)                                                                                    \
  }

#define SWITCH_PUBLICATION_STYLE_INT(dragFunc, sliderFunc)                                                             \
  switch (publication.style) {                                                                                         \
  case Publication::Style::DRAG:                                                                                       \
    DRAW_DRAG_INT(dragFunc, publication.flags &Publication::Flags::RANGE);                                             \
    break;                                                                                                             \
  case Publication::Style::SLIDER:                                                                                     \
    DRAW_SLIDER_INT(sliderFunc, publication.flags &Publication::Flags::RANGE);                                         \
    break;                                                                                                             \
  case Publication::Style::STEPPER:                                                                                    \
    ENGINE_WARNING("Publication labelled {} was styled as STEPPER, which has not yet been implemented!",               \
                   publication.label)                                                                                  \
    break;                                                                                                             \
  default:                                                                                                             \
    ENGINE_ERROR("Publication labelled {} has been given an inväalid style!", publication.label)                       \
    break;                                                                                                             \
  }

#define SWITCH_PUBLICATION_STYLE_FLOAT(dragFunc, sliderFunc)                                                           \
  switch (publication.style) {                                                                                         \
  case Publication::Style::DRAG:                                                                                       \
    DRAW_DRAG_FLOAT(dragFunc, publication.flags &Publication::Flags::RANGE);                                           \
    break;                                                                                                             \
  case Publication::Style::SLIDER:                                                                                     \
    DRAW_SLIDER_FLOAT(sliderFunc, publication.flags &Publication::Flags::RANGE);                                       \
    break;                                                                                                             \
  case Publication::Style::STEPPER:                                                                                    \
    ENGINE_WARNING("Publication labelled {} was styled as STEPPER, which has not yet been implemented!",               \
                   publication.label)                                                                                  \
    break;                                                                                                             \
  default:                                                                                                             \
    ENGINE_ERROR("Publication labelled {} has been given an inväalid style!", publication.label)                       \
    break;                                                                                                             \
  }

void DrawPublication(Publication &publication) {
  switch (publication.type) {
  case Publication::Type::INTEGER1:
    SWITCH_PUBLICATION_STYLE_INT(ImGui::DragInt, ImGui::SliderInt)
    break;

  case Publication::Type::INTEGER2:
    SWITCH_PUBLICATION_STYLE_INT(ImGui::DragInt2, ImGui::SliderInt2)
    break;

  case Publication::Type::INTEGER3:
    SWITCH_PUBLICATION_STYLE_INT(ImGui::DragInt3, ImGui::SliderInt3)
    break;

  case Publication::Type::INTEGER4:
    SWITCH_PUBLICATION_STYLE_INT(ImGui::DragInt4, ImGui::SliderInt4)
    break;

  case Publication::Type::FLOAT1:
    SWITCH_PUBLICATION_STYLE_FLOAT(ImGui::DragFloat, ImGui::SliderFloat)
    break;

  case Publication::Type::FLOAT2:
    SWITCH_PUBLICATION_STYLE_FLOAT(ImGui::DragFloat2, ImGui::SliderFloat2)
    break;

  case Publication::Type::FLOAT3:
    SWITCH_PUBLICATION_STYLE_FLOAT(ImGui::DragFloat3, ImGui::SliderFloat3)
    break;

  case Publication::Type::FLOAT4:
    SWITCH_PUBLICATION_STYLE_FLOAT(ImGui::DragFloat4, ImGui::SliderFloat4)
    break;

  case Publication::Type::TEXT:
    ImGui::Text(publication.label);
    break;
  case Publication::Type::COLOUR_PICKER:
    ImGui::ColorEdit4(publication.label, (float *)publication.referencedPointer, ImGuiColorEditFlags_NoInputs);
    break;
  case Publication::Type::TEXTURE_SELECT:
    ENGINE_WARNING("Publication labelled {} has Type COLOUR_PICKER, which has not yet been implemented!",
                   publication.label)
    break;
  case Publication::Type::SHADER_SELECT:
    ENGINE_WARNING("Publication labelled {} has Type COLOUR_PICKER, which has not yet been implemented!",
                   publication.label)
    break;
  case Publication::Type::MESH_SELECT:
    ENGINE_WARNING("Publication labelled {} has Type COLOUR_PICKER, which has not yet been implemented!",
                   publication.label)
    break;
  case Publication::Type::PREFAB_SELECT:
    ENGINE_WARNING("Publication labelled {} has Type COLOUR_PICKER, which has not yet been implemented!",
                   publication.label)
    break;
  case Publication::Type::MATERIAL_SELECT:
    ENGINE_WARNING("Publication labelled {} has Type COLOUR_PICKER, which has not yet been implemented!",
                   publication.label)
    break;
  case Publication::Type::ENUM:
    ENGINE_WARNING("Publication labelled {} has Type COLOUR_PICKER, which has not yet been implemented!",
                   publication.label)
    break;
  case Publication::Type::COMPOSITE:
    if (ImGui::TreeNodeEx(publication.label, ImGuiTreeNodeFlags_DefaultOpen)) {
      for (auto &P : reinterpret_cast<Publishable *>(publication.referencedPointer)->GetPublications()) {
        DrawPublication(P);
      }
      ImGui::TreePop();
    }
    break;
  default:
    break;
  }
}

void DrawPublishable(Publishable *publishable) {}

void ImGUIManager::BeginFrame() {
  PROFILE_FUNCTION()
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  if (ImGui::Begin("Debug GUI")) {
    Renderer::GetImGUISection();
    ImGui::Text("FPS: %.1f", 1.0f / Time::deltaTime);

    Publication p{.label = "Test", .type = Publication::Type::COMPOSITE, .referencedPointer = &testPublishable};

    DrawPublication(p);

    ImGui::End();
  }

  ImGui::Render();
}

void ImGUIManager::ImGUIDrawCommand::QueueExecution(VkCommandBuffer const &queue) const {
  VkRenderingAttachmentInfo colourAttachment = vkinit::ColourAttachmentInfo(targetView);
  VkRenderingInfo renderInfo = vkinit::RenderingInfo(colourAttachment, targetExtent);

  vkCmdBeginRendering(queue, &renderInfo);

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), queue);

  vkCmdEndRendering(queue);
}

} // namespace Engine::Graphics
