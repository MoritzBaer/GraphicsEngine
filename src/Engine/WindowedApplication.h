#pragma once

#include "Debug/Logging.h"
#include "Game.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/DescriptorHandling.h"
#include "Graphics/RenderTargetProvider.h"
#include "WindowManager.h"

using namespace Engine::Graphics;
using namespace Engine;

struct SwapChainProvider : public Engine::Graphics::RenderResourceProvider {
private:
  InstanceManager const *instanceManager;
  GPUObjectManager *gpuObjectManager;

  static const uint32_t MAX_FRAME_OVERLAP = 3;

  VkQueue graphicsQueue;
  VkQueue presentQueue;

  struct FrameResources {
    CommandQueue commandQueue;
    VkSemaphore presentSemaphore;
    VkSemaphore renderSemaphore;
    VkFence renderFence;
    DescriptorAllocator descriptorAllocator;
    Buffer<DrawData> uniformBuffer;
  };

  VkSwapchainKHR swapchain;
  VkFormat swapchainFormat;
  VkExtent2D swapchainExtent;
  std::vector<Image<2>> swapchainImages;
  std::array<FrameResources, MAX_FRAME_OVERLAP> frameResources;
  uint32_t currentFrame = 0;

  Maths::Dimension2 windowDimension;
  Maths::Dimension2 renderBufferDimension{1600, 900};
  float renderScale = 1.0f;
  DescriptorAllocator descriptorAllocator;
  DescriptorLayoutBuilder descriptorLayoutBuilder;
  DescriptorWriter descriptorWriter;
  VkDescriptorSetLayout singleTextureDescriptorLayout;

  uint32_t resourceIndex;
  uint32_t swapchainImageIndex;

  void CreateSwapchain();
  void DestroySwapchain();

  void CreateFrameResources(FrameResources &resources);
  void DestroyFrameResources(FrameResources &resources);

  RenderResources GetRenderResources() override;
  void DisplayRenderTarget() override; // TODO: Present current swapchain image
  std::vector<Command const *> PrepareTargetForRendering() override { return {}; }
  std::vector<Command const *> PrepareTargetForDisplaying() override;

  inline void RecreateSwapchain() {
    instanceManager->WaitUntilDeviceIdle();
    DestroySwapchain();
    CreateSwapchain();
  }

public:
  SwapChainProvider(InstanceManager const *instanceManager, GPUObjectManager *gpuObjectManager,
                    Maths::Dimension2 const &windowSize);

  inline void SetWindowSize(Maths::Dimension2 const &newSize) {
    windowDimension = newSize;
    RecreateSwapchain();
  }

  VkFormat GetSwapchainFormat() const { return swapchainFormat; }
};

class WindowedApplication {
private:
  Engine::Window *mainWindow;
  Engine::Graphics::VulkanSuite vulkan;
  SwapChainProvider swapChainProvider;

public:
  WindowedApplication(const char *name, Engine::Maths::Dimension2 const &windowSize);
  ~WindowedApplication() { Engine::WindowManager::DestroyWindow(mainWindow); };

  inline Engine::Window *GetWindow() { return mainWindow; }
  inline Engine::Graphics::VulkanSuite
#ifdef NDEBUG
      const
#endif
          *
          GetVulkan()
#ifdef NDEBUG
              const
#endif
  {
    return &vulkan;
  }
  inline SwapChainProvider *GetSwapChainProvider() { return &swapChainProvider; }
};

template <typename GameType> class GameApp {
protected:
  WindowedApplication windowedApplication;
  GameType game;
  const char *name;

public:
  template <typename... GameArgs>
  GameApp(const char *name, Engine::Maths::Dimension2 const &windowSize, GameArgs &&...gameArgs)
      : game(windowedApplication.GetVulkan(), std::forward<GameArgs>(gameArgs)...),
        windowedApplication(name, windowSize) {
    game.renderer.SetFrameResourceProvider(windowedApplication.GetSwapChainProvider());
    windowedApplication.GetWindow()->SetCloseCallback([this]() { game.running = false; });
    windowedApplication.GetWindow()->SetMinimizeCallback([this]() { game.rendering = false; });
    windowedApplication.GetWindow()->SetRestoreCallback([this]() { game.rendering = true; });
    windowedApplication.GetWindow()->SetResizeCallback([this](Engine::Maths::Dimension2 newWindowSize) {
      windowedApplication.GetSwapChainProvider()->SetWindowSize(newWindowSize);
    });
  }

  void Run();
};

template <typename GameType> inline void GameApp<GameType>::Run() {
  try {
    game.Init();

    while (game.IsRunning())
      game.CalculateFrame();
  } catch (std::exception &e) {
    Engine::Debug::Logging::PrintError(name, "Exception: {}", e.what());
  }
}
