#include "Window.h"

#include "Util/Macros.h"
#include "Debug/Logging.h"

namespace Engine
{
Window::Window(uint32_t width, uint32_t height, const char *title)
: width(width), height(height)
{
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

Window::~Window()
{
    glfwDestroyWindow(window);
}

void Window::CreateSurfaceOnWindow(VkInstance instance, VkSurfaceKHR *surface) const
{
    VULKAN_ASSERT(glfwCreateWindowSurface(instance, window, nullptr, surface), "Failed to create surface!")
}

Maths::Dimension2 Window::GetCanvasSize() const
{
    int x, y;
    glfwGetFramebufferSize(window, &x, &y);
    return Maths::Dimension<2>{static_cast<uint32_t>(x), static_cast<uint32_t>(y)};
}
} // namespace Engine
