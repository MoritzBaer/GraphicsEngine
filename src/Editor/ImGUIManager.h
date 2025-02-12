#pragma once

#include "Graphics/CommandQueue.h"
#include "Graphics/Image.h"
#include "Util/Macros.h"
#include "Window.h"

// TODO: Maybe shift to editor?

namespace Engine::Graphics {
struct ImGUIView;

class ImGUIManager {
  InstanceManager const *instanceManager;
  bool showImGuiDemo;

public:
  ImGUIManager(InstanceManager const *instanceManager);
  ~ImGUIManager();

private:
  VkDescriptorPool imGUIPool;
  std::vector<ImGUIView *> views;

public:
  void InitImGUIOnWindow(Window const *window, VkFormat swapchainFormat);

  void BeginFrame();
  void RegisterView(ImGUIView *view);

  class ImGUIDrawCommand : public Command {
    Image<2> const &targetImage;

  public:
    ImGUIDrawCommand(Image<2> const &targetImage) : targetImage(targetImage) {}
    void QueueExecution(VkCommandBuffer const &queue) const;
  } DrawFrameCommand(Image<2> const &targetImage) {
    return ImGUIDrawCommand(targetImage);
  }
};

struct ImGUIView {
  virtual void Draw() = 0;
};

} // namespace Engine::Graphics
