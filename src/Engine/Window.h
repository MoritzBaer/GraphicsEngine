#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"
#include "Maths/Dimension.h"
#include <unordered_map>
#include <functional>

#define CALLBACK_WITH_SETTER(name, ...)                                                                                         \
    private: std::function<void(__VA_ARGS__)> name##Callback;                                                                   \
    public: inline void Set##name##Callback(std::function<void(__VA_ARGS__)> const & callback) { name##Callback = callback; }   \
    private:


namespace Engine
{
    class Window
    {
    public:
        enum class UserEvent
        {
            SHOULD_CLOSE,
            WAS_MINIMIZED,
            WAS_RESTORED
        };

    private:
        GLFWwindow *window;
        uint32_t width;
        uint32_t height;
        bool minimized;

    public:
        Window(uint32_t width, uint32_t height, const char *title);
        ~Window();

        void CallCallbacks();

        void CreateSurfaceOnWindow(VkInstance instance, VkSurfaceKHR *surface) const;
        void InitImGUIOnWindow() const;
        Maths::Dimension2 GetCanvasSize() const;

        // Callbacks
        CALLBACK_WITH_SETTER(Close)
        CALLBACK_WITH_SETTER(Minimize)
        CALLBACK_WITH_SETTER(Restore)
    };
} // namespace Engine::Graphics
