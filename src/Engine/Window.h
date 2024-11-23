#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "Maths/Dimension.h"
#include "vulkan/vulkan.h"
#include <functional>
#include <unordered_map>

#define CALLBACK_WITH_SETTER(name, ...)                                                                                \
private:                                                                                                               \
  std::function<void(__VA_ARGS__)> name##Callback;                                                                     \
                                                                                                                       \
public:                                                                                                                \
  inline void Set##name##Callback(std::function<void(__VA_ARGS__)> const &callback) { name##Callback = callback; }     \
                                                                                                                       \
private:

namespace Engine {
class WindowManager;

class Window {
public:
  enum class UserEvent { SHOULD_CLOSE, WAS_MINIMIZED, WAS_RESTORED };
  friend class WindowManager;

private:
  GLFWwindow *glfwWindow;
  uint32_t width;
  uint32_t height;
  bool minimized;

  Window(GLFWwindow *glfwWindow) : glfwWindow(glfwWindow) {}

  template <WindowManager *windowManager> static void GLFWResizeCallback(GLFWwindow *glfwWindow, int width, int height);

public:
  Window(uint32_t width, uint32_t height, const char *title);
  ~Window();

  void CallCallbacks();

  void CreateSurfaceOnWindow(VkInstance instance, VkSurfaceKHR *surface) const;
  void InitImGUIOnWindow() const;
  Maths::Dimension2 GetCanvasSize() const;

  void CallResizeIfCorrectWindow(Window const *window, Maths::Dimension2 const &size);

  // Callbacks
  CALLBACK_WITH_SETTER(Close)
  CALLBACK_WITH_SETTER(Minimize)
  CALLBACK_WITH_SETTER(Restore)
  CALLBACK_WITH_SETTER(Resize, Maths::Dimension2 const &newSize)
};
} // namespace Engine
