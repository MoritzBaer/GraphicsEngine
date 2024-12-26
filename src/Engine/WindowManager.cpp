#include "WindowManager.h"

#include "Debug/Logging.h"
#include "GLFW/glfw3.h"
#include "Util/Macros.h"

namespace Engine {
void WindowManager::DeleteAllWindows() {
  while (!instance->openWindows.empty()) {
    Window *window = openWindows.back();
    openWindows.pop_back();
    delete window;
  }
}

WindowManager::WindowManager() : openWindows() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

WindowManager::~WindowManager() {
  DeleteAllWindows();
  glfwTerminate();
}

void WindowManager::Init() { instance = new WindowManager(); }
void WindowManager::Cleanup() {
  for (auto w : instance->openWindows) {
    DestroyWindow(w);
  }
  delete instance;
  instance = nullptr;
}

Window *WindowManager::CreateWindow(uint32_t width, uint32_t height, const char *title) {
  if (instance == nullptr) {
    Init();
  }
  instance->openWindows.push_back(new Window(width, height, title));
  return instance->openWindows.back();
}

void WindowManager::DestroyWindow(Window *window) {
  instance->openWindows.erase(std::find(instance->openWindows.begin(), instance->openWindows.end(), window));
  delete window;
}

void WindowManager::HandleEventsOnAllWindows() {
  glfwPollEvents();
  for (auto w : instance->openWindows) {
    w->CallCallbacks();
  }
}
} // namespace Engine