#pragma once

#include "CommandQueue.h"
#include "Image.h"
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
    Image<2> const &targetImage;

  public:
    ImGUIDrawCommand(Image<2> const &targetImage) : targetImage(targetImage) {}
    void QueueExecution(VkCommandBuffer const &queue) const;
  } DrawFrameCommand(Image<2> const &targetImage) {
    return ImGUIDrawCommand(targetImage);
  }
};

struct ImGUIView {
  virtual void Draw() const = 0;
  ImGUIView() { ImGUIManager::RegisterView(this); }
};

} // namespace Engine::Graphics
