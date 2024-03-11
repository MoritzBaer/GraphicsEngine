#include "Window.h"

#include "Util/Macros.h"
#include "Debug/Logging.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

namespace Engine
{
Window::Window(uint32_t width, uint32_t height, const char *title)
: width(width), height(height), minimized(false)
{
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

Window::~Window()
{
    glfwDestroyWindow(window);
}

void Window::CallCallbacks()
{
    Maths::Dimension2 framebufferSize;
    glfwGetFramebufferSize(window, reinterpret_cast<int *>(&framebufferSize[X]), reinterpret_cast<int *>(&framebufferSize[Y]));

    bool newMinimized = framebufferSize.x() == 0 || framebufferSize.y() == 0;

    if (glfwWindowShouldClose(window)) { CloseCallback(); }
    if (newMinimized && !minimized) { MinimizeCallback(); }
    if (!newMinimized && minimized) { RestoreCallback(); }

    minimized = newMinimized;
}

void Window::CreateSurfaceOnWindow(VkInstance instance, VkSurfaceKHR *surface) const
{
    VULKAN_ASSERT(glfwCreateWindowSurface(instance, window, nullptr, surface), "Failed to create surface!")
}

void Window::InitImGUIOnWindow() const
{
    ImGui_ImplGlfw_InitForVulkan(window, true);
}

Maths::Dimension2 Window::GetCanvasSize() const
{
    int x, y;
    glfwGetFramebufferSize(window, &x, &y);
    return Maths::Dimension<2>{static_cast<uint32_t>(x), static_cast<uint32_t>(y)};
}
} // namespace Engine
