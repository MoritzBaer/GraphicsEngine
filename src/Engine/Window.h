#pragma once

#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"

namespace Engine
{
    class Window
    {
    private:
        GLFWwindow *window;
        uint32_t width;
        uint32_t height;
    public:
        Window(uint32_t width, uint32_t height, const char* title);
        ~Window();
        inline bool ShouldClose() const { return glfwWindowShouldClose(window); }
    };
    
} // namespace Engine::Graphics
