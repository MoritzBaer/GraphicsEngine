#pragma once

#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"

namespace Engine
{
    class Window
    {
        friend class WindowManager;
    private:
        GLFWwindow *window;
        uint32_t width;
        uint32_t height;
        uint32_t windowId;
    public:
        Window(uint32_t width, uint32_t height, const char* title);
        ~Window();
        inline bool ShouldClose() const { return glfwWindowShouldClose(window); }
        inline GLFWwindow * GetGLFWwindow() const { return window; }

    };
    
} // namespace Engine::Graphics
