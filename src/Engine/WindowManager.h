#pragma once

#include "Util/Macros.h"
#include "Window.h"
#include <vector>

namespace Engine {
class WindowManager {
  _SINGLETON(WindowManager)

  std::vector<Window *> openWindows;

  void DeleteAllWindows();

public:
  static Window *CreateWindow(Maths::Dimension2 const &windowSize, const char *title);
  static void DestroyWindow(Window *window);

  inline static void CallResizeCallbackOnCorrectWindow(Window const *window, Maths::Dimension2 const &size) {
    for (auto w : instance->openWindows) {
      w->CallResizeIfCorrectWindow(window, size);
    }
  }
  inline static void CallResizeCallbackOnCorrectWindow(GLFWwindow *glfwWindow, int width, int height) {
    CallResizeCallbackOnCorrectWindow(new Window(glfwWindow), Maths::Dimension2(width, height));
  }

  static void HandleEventsOnAllWindows();
};

} // namespace Engine
