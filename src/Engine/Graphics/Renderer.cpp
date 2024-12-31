#include "Renderer.h"
#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "GLFW/glfw3.h"
#include "Graphics/ImGUIManager.h"
#include "UniformAggregate.h"
#include "VulkanUtil.h"
#include <algorithm>
#include <vector>

namespace Engine::Graphics {
class ExecuteComputePipelineCommand : public Command {
  vkutil::BindPipelineCommand bindPipeline;
  vkutil::BindDescriptorSetsCommand bindDescriptors;
  vkutil::DispatchCommand dispatch;
  vkutil::PushConstantsCommand<ComputePushConstants> pushConstants;

public:
  ExecuteComputePipelineCommand(VkPipeline const &pipeline, VkPipelineBindPoint const &bindPoint,
                                VkPipelineLayout const &layout, std::vector<VkDescriptorSet> const &descriptors,
                                ComputePushConstants const &pushConstants, uint32_t workerGroupsX,
                                uint32_t workerGroupsY, uint32_t workerGroupsZ)
      : bindPipeline(pipeline, bindPoint), bindDescriptors(bindPoint, layout, descriptors),
        dispatch(workerGroupsX, workerGroupsY, workerGroupsZ), pushConstants(pushConstants, layout) {}
  ExecuteComputePipelineCommand(VkPipeline const &pipeline, VkPipelineBindPoint const &bindPoint,
                                VkPipelineLayout const &layout, VkDescriptorSet const &descriptor,
                                ComputePushConstants const &pushConstants, uint32_t workerGroupsX,
                                uint32_t workerGroupsY, uint32_t workerGroupsZ)
      : bindPipeline(pipeline, bindPoint), bindDescriptors(bindPoint, layout, descriptor),
        pushConstants(pushConstants, layout), dispatch(workerGroupsX, workerGroupsY, workerGroupsZ) {}
  ExecuteComputePipelineCommand(Renderer::CompiledEffect const &effect, VkPipelineBindPoint const &bindPoint,
                                VkDescriptorSet const &descriptor, uint32_t workerGroupsX, uint32_t workerGroupsY,
                                uint32_t workerGroupsZ)
      : bindPipeline(effect.pipeline, bindPoint), bindDescriptors(bindPoint, effect.pipelineLayout, descriptor),
        pushConstants(effect.data, effect.pipelineLayout), dispatch(workerGroupsX, workerGroupsY, workerGroupsZ) {}
  inline void QueueExecution(VkCommandBuffer const &queue) const {
    bindPipeline.QueueExecution(queue);
    bindDescriptors.QueueExecution(queue);
    pushConstants.QueueExecution(queue);
    dispatch.QueueExecution(queue);
  }
};

class ImGUIDrawCommand : public Command {
  Image2 const &targetImage;

public:
  ImGUIDrawCommand(Image2 const &targetImage) : targetImage(targetImage) {}
  void QueueExecution(VkCommandBuffer const &queue) const;
};

class DrawGeometryCommand : public Command {
  VkImageView drawImageView;
  VkImageView depthImageView;
  VkExtent2D drawExtent;
  VkPipeline graphicsPipeline;
  // TODO: Add geometry data as well
public:
  DrawGeometryCommand(VkImageView const &view, VkExtent2D const &extent, VkImageView const &depthView,
                      VkPipeline const &graphicsPipeline)
      : drawImageView(view), depthImageView(depthView), drawExtent(extent), graphicsPipeline(graphicsPipeline) {}
  void QueueExecution(VkCommandBuffer const &) const;
};

class MultimeshDrawCommand : public Command {
  Image<2> const &drawImage;
  Image<2> const &depthImage;
  Maths::Dimension2 renderAreaSize;
  Maths::Dimension2 renderAreaOffset;
  DescriptorAllocator &descriptorAllocator;
  DescriptorWriter &descriptorWriter;
  Buffer<DrawData> uniformBuffer;
  std::span<MeshRenderer const *> singleMeshes;

public:
  MultimeshDrawCommand(Image<2> const &drawImage, Image<2> const &depthImage, DescriptorAllocator &descriptorAllocator,
                       DescriptorWriter &descriptorWriter, Maths::Dimension2 const &renderAreaSize,
                       Buffer<DrawData> const &uniformBuffer, std::span<MeshRenderer const *> const &meshes)
      : drawImage(drawImage), depthImage(depthImage), singleMeshes(meshes), uniformBuffer(uniformBuffer),
        renderAreaSize(renderAreaSize), descriptorAllocator(descriptorAllocator), descriptorWriter(descriptorWriter) {}
  void QueueExecution(VkCommandBuffer const &) const;
};

Renderer::Renderer(Maths::Dimension2 const &windowSize, InstanceManager const *instanceManager,
                   GPUObjectManager *gpuObjectManager,
                   std::vector<ComputeEffect<ComputePushConstants>> const &uncompiledBackgroundEffects)
    : instanceManager(instanceManager), descriptorLayoutBuilder(instanceManager), descriptorWriter(instanceManager),
      gpuObjectManager(gpuObjectManager), backgroundEffects(uncompiledBackgroundEffects.size()) {
  PROFILE_FUNCTION()
  instanceManager->GetGraphicsQueue(&graphicsQueue);
  instanceManager->GetPresentQueue(&presentQueue);
  windowDimension = windowSize;
  renderBufferInitialized = false;
  CreateSwapchain();
  RecreateRenderBuffer();
  for (int i = 0; i < MAX_FRAME_OVERLAP; i++) {
    CreateFrameResources(frameResources[i]);
  }
  InitDescriptors();
  CompileBackgroundEffects(uncompiledBackgroundEffects);
}

void Renderer::CompileBackgroundEffects(std::vector<ComputeEffect<ComputePushConstants>> const &uncompiledEffects) {
  PROFILE_FUNCTION()
  VkPushConstantRange pushConstants{
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT, .offset = 0, .size = sizeof(ComputePushConstants)};

  VkPipelineLayoutCreateInfo computeLayoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                               .setLayoutCount = 1,
                                               .pSetLayouts = &renderBufferDescriptorLayout,
                                               .pushConstantRangeCount = 1,
                                               .pPushConstantRanges = &pushConstants};

  VkPipelineLayout computePipelineLayout;
  instanceManager->CreatePipelineLayout(&computeLayoutInfo, &computePipelineLayout);

  VkComputePipelineCreateInfo pipelineInfo{.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                                           .layout = computePipelineLayout};

  for (int i = 0; i < uncompiledEffects.size(); i++) {
    pipelineInfo.stage = uncompiledEffects[i].effectShader.GetStageInfo();

    backgroundEffects[i].name = uncompiledEffects[i].name.c_str();
    backgroundEffects[i].pipelineLayout = computePipelineLayout;
    backgroundEffects[i].data = uncompiledEffects[i].constants;
    instanceManager->CreateComputePipeline(pipelineInfo, &backgroundEffects[i].pipeline);
  }
}

Renderer::~Renderer() {
  for (auto effect : backgroundEffects) {
    instanceManager->DestroyPipeline(effect.pipeline);
  }
  instanceManager->DestroyPipelineLayout(backgroundEffects[0].pipelineLayout);
  DestroySwapchain();
  for (int i = 0; i < MAX_FRAME_OVERLAP; i++) {
    DestroyFrameResources(frameResources[i]);
  }
  DestroyRenderBuffer();
  descriptorAllocator.ClearDescriptors();
  descriptorAllocator.DestroyPools();
  instanceManager->DestroyDescriptorSetLayout(renderBufferDescriptorLayout);
}

VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities, Maths::Dimension2 canvasSize) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    VkExtent2D actualExtent = {
        std::clamp(canvasSize[X], capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp(canvasSize[Y], capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};

    return actualExtent;
  }
}

void Renderer::CreateSwapchain() {
  PROFILE_FUNCTION()

  SwapchainSupportDetails details = instanceManager->GetSwapchainSupport();

  VkSurfaceFormatKHR surfaceFormat = ChooseSwapchainSurfaceFormat(details.formats);
  VkPresentModeKHR presentMode = ChooseSwapchainPresentMode(details.presentModes);
  VkExtent2D extent = ChooseSwapchainExtent(details.capabilities, windowDimension);

  uint32_t imageCount =
      std::min(std::max(details.capabilities.minImageCount + 1, MAX_FRAME_OVERLAP), details.capabilities.maxImageCount);

  instanceManager->CreateSwapchain(surfaceFormat, presentMode, extent, imageCount, VK_NULL_HANDLE, &swapchain);

  swapchainExtent = extent;
  swapchainFormat = surfaceFormat.format;

  std::vector<VkImage> scImgs;
  instanceManager->GetSwapchainImages(swapchain, scImgs);
  swapchainImages.resize(scImgs.size());

  // Create image views
  for (int i = 0; i < swapchainImages.size(); i++) {
    swapchainImages[i] = gpuObjectManager->CreateImage(scImgs[i], windowDimension, surfaceFormat.format,
                                                       VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_ASPECT_COLOR_BIT);
  }
}

void Renderer::DestroySwapchain() {
  for (auto image : swapchainImages) {
    gpuObjectManager->DestroyImage(image);
  }
  instanceManager->DestroySwapchain(swapchain);
}

void Renderer::DrawFrame(Camera const *camera, SceneData const &sceneData,
                         std::span<MeshRenderer const *> const &objectsToDraw) {

  PROFILE_FUNCTION()
  uint32_t resourceIndex = currentFrame % MAX_FRAME_OVERLAP;

  VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
  {
    PROFILE_SCOPE("Waiting for previous frame to finish rendering")
    instanceManager->WaitForFences(&frameResources[resourceIndex].renderFence);
    instanceManager->ResetFences(&frameResources[resourceIndex].renderFence);
  }
  VkResult swapchainImageAcqusitionResult;
  uint32_t swapchainImageIndex = instanceManager->GetNextSwapchainImageIndex(
      swapchainImageAcqusitionResult, swapchain, frameResources[resourceIndex].swapchainSemaphore);
  if (swapchainImageAcqusitionResult == VK_ERROR_OUT_OF_DATE_KHR ||
      swapchainImageAcqusitionResult == VK_SUBOPTIMAL_KHR) {
    RecreateSwapchain();
    return;
  }

  frameResources[resourceIndex].descriptorAllocator.ClearDescriptors();

  {
    PROFILE_SCOPE("Generate commands")

    // Commands for drawing background
    auto transitionBufferToWriteable = renderBuffer.colourImage.Transition(VK_IMAGE_LAYOUT_GENERAL);
    auto computeRun = ExecuteComputePipelineCommand(
        backgroundEffects[currentBackgroundEffect], VK_PIPELINE_BIND_POINT_COMPUTE, renderBufferDescriptors,
        std::ceil<uint32_t>(windowDimension[X] / 16u), std::ceil<uint32_t>(windowDimension[Y] / 16u), 1);

    // Commands for drawing geometry
    auto transitionBufferToRenderTarget = renderBuffer.colourImage.Transition(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    auto transitionBufferToDepthStencil =
        renderBuffer.depthImage.Transition(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    Dimension2 scaledDrawDimension = Dimension2(std::min(windowDimension.x(), renderBufferDimension.x()),
                                                std::min(windowDimension.y(), renderBufferDimension.y())) *
                                     renderScale;

    Maths::Matrix4 view = camera->entity.GetComponent<Transform>()->WorldToModelMatrix();
    Maths::Matrix4 projection = camera->projection;

    DrawData uniformData{
        .view = view,
        .projection = projection,
        .viewProjection = projection * view,
        .sceneData = sceneData,
    };

    frameResources[resourceIndex].uniformBuffer.SetData(uniformData);
    DescriptorWriter descriptorWriter{instanceManager};
    auto drawMeshes = MultimeshDrawCommand(
        renderBuffer.colourImage, renderBuffer.depthImage, frameResources[resourceIndex].descriptorAllocator,
        descriptorWriter, scaledDrawDimension, frameResources[resourceIndex].uniformBuffer, objectsToDraw);

    // Commands for copying render to swapchain
    auto transitionBufferToTransferSrc = renderBuffer.colourImage.Transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    auto transitionPresenterToTransferDst =
        swapchainImages[swapchainImageIndex].Transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    auto copyBufferToPresenter = renderBuffer.colourImage.BlitTo(swapchainImages[swapchainImageIndex]);

    // Commands for rendering ImGUI
    auto transitionPresenterToColourAttachment =
        swapchainImages[swapchainImageIndex].Transition(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    auto renderImGUI = ImGUIDrawCommand(swapchainImages[swapchainImageIndex]);

    // Command for presenting
    auto transitionPresenterToPresent =
        swapchainImages[swapchainImageIndex].Transition(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    commandBufferSubmitInfo = frameResources[resourceIndex].commandQueue.EnqueueCommandSequence(
        {// Draw background
         &transitionBufferToWriteable, &computeRun,

         // Draw geometry
         &transitionBufferToRenderTarget, &transitionBufferToDepthStencil, &drawMeshes,

         // Copy render to swapchain
         &transitionBufferToTransferSrc, &transitionPresenterToTransferDst, &copyBufferToPresenter,

         // Render ImGui directly to swapchain
         &transitionPresenterToColourAttachment, &renderImGUI, &transitionPresenterToPresent});
  }

  auto semaphoreWaitInfo = vkinit::SemaphoreSubmitInfo(frameResources[resourceIndex].swapchainSemaphore,
                                                       VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
  auto semaphoreSignalInfo =
      vkinit::SemaphoreSubmitInfo(frameResources[resourceIndex].renderSemaphore, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);

  VkSubmitInfo2 submitInfo = vkinit::SubmitInfo(semaphoreWaitInfo, commandBufferSubmitInfo, semaphoreSignalInfo);

  VULKAN_ASSERT(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, frameResources[resourceIndex].renderFence),
                "Failed to submit queue")

  VkPresentInfoKHR presentInfo{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                               .waitSemaphoreCount = 1,
                               .pWaitSemaphores = &frameResources[resourceIndex].renderSemaphore,
                               .swapchainCount = 1,
                               .pSwapchains = &swapchain,
                               .pImageIndices = &swapchainImageIndex};

  VULKAN_ASSERT(vkQueuePresentKHR(presentQueue, &presentInfo), "Failed to present image!")

  frameResources[resourceIndex].deletionQueue.Flush();
  currentFrame++;
}

void Renderer::DestroyRenderBuffer() {
  gpuObjectManager->DestroyAllocatedImage(renderBuffer.colourImage);
  gpuObjectManager->DestroyAllocatedImage(renderBuffer.depthImage);
  renderBufferInitialized = false;
}

void Renderer::CreateFrameResources(FrameResources &resources) {

  VkFenceCreateInfo fenceInfo = vkinit::FenceCreateInfo();

  VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

  instanceManager->CreateFence(&fenceInfo, &resources.renderFence);
  instanceManager->CreateSemaphore(&semaphoreInfo, &resources.renderSemaphore);
  instanceManager->CreateSemaphore(&semaphoreInfo, &resources.swapchainSemaphore);

  resources.commandQueue = gpuObjectManager->CreateCommandQueue();

  std::vector<DescriptorAllocator::PoolSizeRatio> frame_sizes = {
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4},
  };

  resources.descriptorAllocator = DescriptorAllocator(instanceManager);
  resources.descriptorAllocator.InitPools(10, frame_sizes);
  resources.uniformBuffer =
      gpuObjectManager->CreateBuffer<DrawData>(1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
}

void Renderer::DestroyFrameResources(FrameResources &resources) {
  gpuObjectManager->DestroyCommandQueue(resources.commandQueue);
  instanceManager->DestroySemaphore(resources.swapchainSemaphore);
  instanceManager->DestroySemaphore(resources.renderSemaphore);
  instanceManager->DestroyFence(resources.renderFence);
  resources.descriptorAllocator.ClearDescriptors();
  resources.descriptorAllocator.DestroyPools();
  gpuObjectManager->DestroyBuffer(resources.uniformBuffer);
}

std::vector<VkFormat> formatsByPreference = {VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_SNORM};

VkImageUsageFlags renderBufferUsage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

VkFormat Renderer::ChooseRenderBufferFormat() {
  VkPhysicalDeviceImageFormatInfo2 formatInfo{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,
                                              .type = VK_IMAGE_TYPE_2D,
                                              .tiling = VK_IMAGE_TILING_OPTIMAL,
                                              .usage = renderBufferUsage};
  for (auto format : formatsByPreference) {
    formatInfo.format = format;
    if (instanceManager->SupportsFormat(formatInfo)) {
      return format;
    }
  }
  ENGINE_ERROR("No suitable format found for render buffer!")
  return VK_FORMAT_UNDEFINED;
}

void Renderer::RecreateRenderBuffer() {
  VkExtent3D renderBufferExtent = {windowDimension.x(), windowDimension.y(), 1};

  if (renderBufferInitialized) {
    DestroyRenderBuffer();
  }
  renderBuffer.colourImage = gpuObjectManager->CreateAllocatedImage(ChooseRenderBufferFormat(), renderBufferDimension,
                                                                    renderBufferUsage, VK_IMAGE_ASPECT_COLOR_BIT);
  renderBuffer.depthImage =
      gpuObjectManager->CreateAllocatedImage(VK_FORMAT_D32_SFLOAT, renderBufferDimension,
                                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Renderer::InitDescriptors() {
  PROFILE_FUNCTION()
  std::vector<DescriptorAllocator::PoolSizeRatio> ratios{{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};

  descriptorAllocator = DescriptorAllocator(instanceManager);
  descriptorAllocator.InitPools(10, ratios);

  descriptorLayoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
  renderBufferDescriptorLayout = descriptorLayoutBuilder.Build(VK_SHADER_STAGE_COMPUTE_BIT);
  descriptorLayoutBuilder.Clear();

  renderBufferDescriptors = descriptorAllocator.Allocate(renderBufferDescriptorLayout);

  descriptorWriter.WriteImage(0, renderBuffer.colourImage, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
  descriptorWriter.UpdateSet(renderBufferDescriptors);
  descriptorWriter.Clear();
}

void Renderer::GetImGUISection() {
  ImGui::BeginGroup();
  ImGui::Text("Background effects");
  if (ImGui::BeginCombo("Choose an effect", backgroundEffects[currentBackgroundEffect].name)) {
    for (int i = 0; i < backgroundEffects.size(); i++) {
      bool isSelected = currentBackgroundEffect == i;
      if (ImGui::Selectable(backgroundEffects[i].name, isSelected)) {
        currentBackgroundEffect = i;
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }

    ImGui::EndCombo();
  }
  ImGui::InputFloat4("data1", (float *)&backgroundEffects[currentBackgroundEffect].data.data1);
  ImGui::InputFloat4("data2", (float *)&backgroundEffects[currentBackgroundEffect].data.data2);
  ImGui::InputFloat4("data3", (float *)&backgroundEffects[currentBackgroundEffect].data.data3);
  ImGui::InputFloat4("data4", (float *)&backgroundEffects[currentBackgroundEffect].data.data4);

  ImGui::SliderFloat("Render scale", &renderScale, 0.1f, 1.0f, "%.3f");

  ImGui::EndGroup();
}

void DrawGeometryCommand::QueueExecution(VkCommandBuffer const &queue) const {
  VkRenderingAttachmentInfo colourAttachmentInfo = vkinit::ColourAttachmentInfo(drawImageView);
  VkRenderingAttachmentInfo depthAttachmentInfo = vkinit::DepthAttachmentInfo(depthImageView);
  VkRenderingInfo renderingInfo = vkinit::RenderingInfo(colourAttachmentInfo, depthAttachmentInfo, drawExtent);

  VkViewport viewport{.x = 0,
                      .y = 0,
                      .width = static_cast<float>(drawExtent.width),
                      .height = static_cast<float>(drawExtent.height),
                      .minDepth = 0.0f,
                      .maxDepth = 1.0f};

  VkRect2D scissor{.offset = {0, 0}, .extent = drawExtent};

  vkCmdBeginRendering(queue, &renderingInfo);
  vkCmdBindPipeline(queue, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
  vkCmdSetViewport(queue, 0, 1, &viewport);
  vkCmdSetScissor(queue, 0, 1, &scissor);

  vkCmdDraw(queue, 3, 1, 0, 0);
  vkCmdEndRendering(queue);
}

void DrawSingleMesh(VkCommandBuffer const &commandBuffer, DescriptorAllocator &descriptorAllocator,
                    DescriptorWriter &descriptorWriter, Buffer<DrawData> const &uniformBuffer,
                    MeshRenderer const *renderInfo) {
  AllocatedMesh const *mesh = renderInfo->mesh;
  Material const *material = renderInfo->material;

  // Bind material pipelines
  VkDescriptorSet descriptorSet = descriptorAllocator.Allocate(material->GetDescriptorSetLayout(1));
  material->Bind(commandBuffer, descriptorAllocator, descriptorWriter, uniformBuffer);

  // Upload uniform data
  Maths::Matrix4 model = renderInfo->entity.GetComponent<Transform>()->ModelToWorldMatrix();
  Maths::Matrix4 normals = renderInfo->entity.GetComponent<Transform>()->ModelToWorldMatrix().Inverse().Transposed();
  PushConstantsAggregate data{};
  data.PushData(&model);
  mesh->AppendData(data);
  material->AppendData(data);

  vkCmdPushConstants(commandBuffer, material->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, data.Size(),
                     data.Data());

  // Draw mesh
  mesh->BindAndDraw(commandBuffer);
}

void MultimeshDrawCommand::QueueExecution(VkCommandBuffer const &queue) const {
  VkRenderingAttachmentInfo colourAttachmentInfo = drawImage.BindAsColourAttachment();
  VkRenderingAttachmentInfo depthAttachmentInfo = depthImage.BindAsDepthAttachment();

  VkExtent2D drawExtent{renderAreaSize.x(), renderAreaSize.y()};
  VkOffset2D drawOffset{static_cast<int32_t>(renderAreaOffset.x()), static_cast<int32_t>(renderAreaOffset.y())};
  VkRenderingInfo renderingInfo = vkinit::RenderingInfo(colourAttachmentInfo, depthAttachmentInfo, drawExtent);

  VkViewport viewport{.x = static_cast<float>(drawOffset.x),
                      .y = static_cast<float>(drawOffset.y),
                      .width = static_cast<float>(drawExtent.width),
                      .height = static_cast<float>(drawExtent.height),
                      .minDepth = 0.0f,
                      .maxDepth = 1.0f};

  VkRect2D scissor{.offset = {2 * drawOffset.x, 2 * drawOffset.y},
                   .extent = {drawExtent.width * 20, drawExtent.height * 20}};

  vkCmdSetViewport(queue, 0, 1, &viewport);
  vkCmdSetScissor(queue, 0, 1, &scissor);

  vkCmdBeginRendering(queue, &renderingInfo);

  for (auto mesh : singleMeshes) {
    DrawSingleMesh(queue, descriptorAllocator, descriptorWriter, uniformBuffer, mesh);
  }

  vkCmdEndRendering(queue);
}

void ImGUIDrawCommand::QueueExecution(VkCommandBuffer const &queue) const {
  auto colourInfo = targetImage.BindAsColourAttachment();
  VkExtent2D extent = {targetImage.GetExtent().x(), targetImage.GetExtent().y()};
  VkRenderingInfo renderInfo = vkinit::RenderingInfo(colourInfo, extent);

  vkCmdBeginRendering(queue, &renderInfo);

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), queue);

  vkCmdEndRendering(queue);
}

} // namespace Engine::Graphics