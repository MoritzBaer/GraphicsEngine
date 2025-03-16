#include "Window.h"

#include "Debug/Logging.h"
#include "Util/Macros.h"
#include "WindowManager.h"
#ifdef USING_IMGUI
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#endif

namespace Engine {

template <WindowManager *windowManager> void Window::GLFWResizeCallback(GLFWwindow *glfwWindow, int width, int height) {
  Window comparison = Window(glfwWindow, nullptr);
  WindowManager::CallResizeCallbackOnCorrectWindow(nullptr, &comparison, Maths::Dimension2(width, height));
}

Window::Window(Maths::Dimension2 const &windowSize, const char *title)
    : canvasSize(windowSize), minimized(false),
      glfwWindow(glfwCreateWindow(windowSize[WIDTH], windowSize[HEIGHT], title, nullptr, nullptr)) {
  glfwSetFramebufferSizeCallback(glfwWindow, &WindowManager::CallResizeCallbackOnCorrectWindow);
}

Window::~Window() { glfwDestroyWindow(glfwWindow); }

void Window::CallCallbacks() {
  Maths::Dimension2 framebufferSize;
  glfwGetFramebufferSize(glfwWindow, reinterpret_cast<int *>(&framebufferSize[X]),
                         reinterpret_cast<int *>(&framebufferSize[Y]));

  bool newMinimized = framebufferSize.x() == 0 || framebufferSize.y() == 0;

  if (glfwWindowShouldClose(glfwWindow)) {
    CloseCallback();
  }
  if (newMinimized && !minimized) {
    MinimizeCallback();
  }
  if (!newMinimized && minimized) {
    RestoreCallback();
  }

  minimized = newMinimized;
}

void Window::CreateSurfaceOnWindow(VkInstance instance, VkSurfaceKHR *surface) const {
  VULKAN_ASSERT(glfwCreateWindowSurface(instance, glfwWindow, nullptr, surface), "Failed to create surface!")
}

#ifdef USING_IMGUI
void Window::InitImGUIOnWindow() const { ImGui_ImplGlfw_InitForVulkan(glfwWindow, true); }
#endif

Maths::Dimension2 Window::GetCanvasSize() const {
  int x, y;
  glfwGetFramebufferSize(glfwWindow, &x, &y);
  return Maths::Dimension<2>{static_cast<uint32_t>(x), static_cast<uint32_t>(y)};
}
void Window::CallResizeIfCorrectWindow(Window const *window, Maths::Dimension2 const &size) {
  if (glfwWindow == window->glfwWindow)
    ResizeCallback(size);
}
} // namespace Engine
