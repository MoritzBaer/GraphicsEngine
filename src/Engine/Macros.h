#pragma once

#define ENGINE_MESSAGE(format, ...) { Debug::Logging::PrintMessage("Engine", format, __VA_ARGS__); }
#define ENGINE_WARNING(format, ...) { Debug::Logging::PrintWarning("Engine", format, __VA_ARGS__); }
#define ENGINE_ERROR(format, ...) { Debug::Logging::PrintError("Engine", format, __VA_ARGS__); __debugbreak(); } 

#define ENGINE_ASSERT(condition, format, ...) if (!condition) { ENGINE_ERROR(format, __VA_ARGS__) }

#define VULKAN_ASSERT(call, format, ...) if (call != VK_SUCCESS) { ENGINE_ERROR(format, __VA_ARGS__) }

#define ENGINE_VERSION VK_MAKE_VERSION(0, 0, 1)

#define _SINGLETON(name, ...)                   \
    private:                                    \
        static inline name* instance = nullptr; \
        name();                                 \
        ~name();                                \
    public:                                     \
        static void Init(__VA_ARGS__);          \
        static void Cleanup();                  \
        name(name &other) = delete;             \
        void operator= (name const &) = delete; \
    private: