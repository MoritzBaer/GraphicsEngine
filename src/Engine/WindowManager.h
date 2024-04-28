#pragma once

#include "Util/Macros.h"
#include "Window.h"
#include <vector>

namespace Engine {
class WindowManager {
  _SINGLETON(WindowManager)

  std::vector<Window *> openWindows;

  inline Window *_CreateWindow(uint32_t width, uint32_t height, const char *title);
  inline void _DestroyWindow(Window *window);
  void _DeleteAllWindows();

public:
  static Window *CreateWindow(uint32_t width, uint32_t height, const char *title);
  static void DestroyWindow(Window *window);
  static inline void CallAllCallbacks() {
    for (auto window : instance->openWindows) {
      window->CallCallbacks();
    }
  }
  static inline void CallResizeCallbackOnCorrectWindow(Window const *window, Maths::Dimension2 const &size) {
    for (auto w : instance->openWindows) {
      w->CallResizeIfCorrectWindow(window, size);
    }
  }
};

} // namespace Engine
