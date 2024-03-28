#pragma once

#include "CommandQueue.h"
#include "Util/Macros.h"
#include "Window.h"

// TODO: Maybe shift to editor?

namespace Engine::Graphics {
struct ImGUIView;

class ImGUIManager {
  _SINGLETON(ImGUIManager, Window const *window)

  VkDescriptorPool imGUIPool;
  std::vector<ImGUIView const *> views;

public:
  static void BeginFrame();
  static void RegisterView(ImGUIView const *view);

  static class ImGUIDrawCommand : public Command {
    VkImageView targetView;
    VkExtent2D targetExtent;

  public:
    ImGUIDrawCommand(VkImageView const &targetView, VkExtent2D const &targetExtent)
        : targetView(targetView), targetExtent(targetExtent) {}
    void QueueExecution(VkCommandBuffer const &queue) const;
  } DrawFrameCommand(VkImageView const &targetView, VkExtent2D const &targetExtent) {
    return ImGUIDrawCommand(targetView, targetExtent);
  }
};

struct ImGUIView {
  virtual void Draw() const = 0;
  ImGUIView() { ImGUIManager::RegisterView(this); }
};

} // namespace Engine::Graphics
