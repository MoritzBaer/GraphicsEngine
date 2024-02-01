#include "Window.h"

Engine::Window::Window(uint32_t width, uint32_t height, const char *title)
: width(width), height(height)
{
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

Engine::Window::~Window()
{
    glfwDestroyWindow(window);
}
