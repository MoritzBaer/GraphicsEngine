#include "WindowedApplication.h"

#include "Debug/Profiling.h"

WindowedApplication::WindowedApplication(const char *name, Maths::Dimension2 const &windowSize)
    : mainWindow(Engine::WindowManager::CreateWindow(windowSize, name)), vulkan(name, mainWindow),
      swapChainProvider(&vulkan.instanceManager, &vulkan.gpuObjectManager, mainWindow->GetCanvasSize()) {}

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

void SwapChainProvider::CreateSwapchain() {
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

void SwapChainProvider::DestroySwapchain() {
  for (auto image : swapchainImages) {
    gpuObjectManager->DestroyImage(image);
  }
  instanceManager->DestroySwapchain(swapchain);
}

void SwapChainProvider::CreateFrameResources(FrameResources &resources) {

  VkFenceCreateInfo fenceInfo = vkinit::FenceCreateInfo();

  VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

  instanceManager->CreateFence(&fenceInfo, &resources.renderFence);
  instanceManager->CreateSemaphore(&semaphoreInfo, &resources.renderSemaphore);
  instanceManager->CreateSemaphore(&semaphoreInfo, &resources.presentSemaphore);

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

void SwapChainProvider::DestroyFrameResources(FrameResources &resources) {
  gpuObjectManager->DestroyCommandQueue(resources.commandQueue);
  instanceManager->DestroySemaphore(resources.presentSemaphore);
  instanceManager->DestroySemaphore(resources.renderSemaphore);
  instanceManager->DestroyFence(resources.renderFence);
  resources.descriptorAllocator.ClearDescriptors();
  resources.descriptorAllocator.DestroyPools();
  gpuObjectManager->DestroyBuffer(resources.uniformBuffer);
}

RenderResourceProvider::RenderResources SwapChainProvider::GetRenderResources() {

  PROFILE_FUNCTION()

  resourceIndex = currentFrame % MAX_FRAME_OVERLAP;

  VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
  {
    PROFILE_SCOPE("Waiting for previous frame to finish rendering")
    instanceManager->WaitForFences(&frameResources[resourceIndex].renderFence);
    instanceManager->ResetFences(&frameResources[resourceIndex].renderFence);
  }

  frameResources[resourceIndex].descriptorAllocator.ClearDescriptors();

  VkResult swapchainImageAcqusitionResult;
  swapchainImageIndex = instanceManager->GetNextSwapchainImageIndex(swapchainImageAcqusitionResult, swapchain,
                                                                    frameResources[resourceIndex].presentSemaphore);
  if (swapchainImageAcqusitionResult == VK_ERROR_OUT_OF_DATE_KHR ||
      swapchainImageAcqusitionResult == VK_SUBOPTIMAL_KHR) {
    RecreateSwapchain();
    return {.commandQueue = {},
            .presentSemaphore = VK_NULL_HANDLE,
            .renderSemaphore = VK_NULL_HANDLE,
            .renderFence = VK_NULL_HANDLE,
            .descriptorAllocator = {},
            .uniformBuffer = {},
            .renderTarget = nullptr};
  }

  return {.commandQueue = frameResources[resourceIndex].commandQueue,
          .presentSemaphore = frameResources[resourceIndex].presentSemaphore,
          .renderSemaphore = frameResources[resourceIndex].renderSemaphore,
          .renderFence = frameResources[resourceIndex].renderFence,
          .descriptorAllocator = frameResources[resourceIndex].descriptorAllocator,
          .uniformBuffer = frameResources[resourceIndex].uniformBuffer,
          .renderTarget = &swapchainImages[swapchainImageIndex]};
}

void SwapChainProvider::DisplayRenderTarget() {

  VkPresentInfoKHR presentInfo{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                               .waitSemaphoreCount = 1,
                               .pWaitSemaphores = &frameResources[currentFrame % MAX_FRAME_OVERLAP].renderSemaphore,
                               .swapchainCount = 1,
                               .pSwapchains = &swapchain,
                               .pImageIndices = &swapchainImageIndex};

  VULKAN_ASSERT(vkQueuePresentKHR(presentQueue, &presentInfo), "Failed to present image!")

  currentFrame++;
}

std::vector<Command const *> SwapChainProvider::PrepareTargetForDisplaying() {
  return {swapchainImages[swapchainImageIndex].Transition(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)};
}

SwapChainProvider::SwapChainProvider(InstanceManager const *instanceManager, GPUObjectManager *gpuObjectManager,
                                     Maths::Dimension2 const &windowSize)
    : instanceManager(instanceManager), descriptorLayoutBuilder(instanceManager), descriptorWriter(instanceManager),
      gpuObjectManager(gpuObjectManager), windowDimension(windowSize), currentFrame(0), swapchainImageIndex(0) {
  PROFILE_FUNCTION()
  instanceManager->GetGraphicsQueue(&graphicsQueue);
  instanceManager->GetPresentQueue(&presentQueue);
  CreateSwapchain();
  for (int i = 0; i < MAX_FRAME_OVERLAP; i++) {
    CreateFrameResources(frameResources[i]);
  }
}
