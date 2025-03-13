#pragma once
#include "vulkan/vk_enum_string_helper.h"

#define __FILE_NAME__ (__FILE__ + SOURCE_PATH_SIZE)

#define ENGINE_MESSAGE(format, ...)                                                                                    \
  {                                                                                                                    \
    Debug::Logging::PrintMessage("Engine", format " ({}({},0))", __VA_OPT__(__VA_ARGS__, ) __FILE_NAME__, __LINE__);   \
  }
#define ENGINE_WARNING(format, ...)                                                                                    \
  {                                                                                                                    \
    Debug::Logging::PrintWarning("Engine", format " ({}({},0))", __VA_OPT__(__VA_ARGS__, ) __FILE_NAME__, __LINE__);   \
  }
#define ENGINE_SUCCESS(format, ...)                                                                                    \
  {                                                                                                                    \
    Debug::Logging::PrintSuccess("Engine", format " ({}({},0))", __VA_OPT__(__VA_ARGS__, ) __FILE_NAME__, __LINE__);   \
  }
#define ENGINE_ERROR(format, ...)                                                                                      \
  {                                                                                                                    \
    Debug::Logging::PrintError("Engine", format " ({}({},0))", __VA_OPT__(__VA_ARGS__, ) __FILE_NAME__, __LINE__);     \
    __debugbreak();                                                                                                    \
  }

#define ENGINE_ASSERT(condition, format, ...)                                                                          \
  if (!(condition)) {                                                                                                  \
    ENGINE_ERROR(format __VA_OPT__(, __VA_ARGS__))                                                                     \
  }

#define _CAT(a, b)                                                                                                     \
  a##b // I don't understand the preprocessor well enough to understand why this is necessary, but apparently it is...
#define CAT(a, b) _CAT(a, b)
#define VAR_WITH_LINE(name) CAT(name, __LINE__)

#define VULKAN_ASSERT(call, format, ...)                                                                               \
  VkResult VAR_WITH_LINE(result) = call;                                                                               \
  if (VAR_WITH_LINE(result) != VK_SUCCESS) {                                                                           \
    ENGINE_ERROR("Vulkan error: {}, message: " format,                                                                 \
                 string_VkResult(VAR_WITH_LINE(result)) __VA_OPT__(, __VA_ARGS__))                                     \
  }

#define ENGINE_VERSION VK_MAKE_VERSION(0, 0, 1)

#define _SINGLETON(name, ...)                                                                                          \
private:                                                                                                               \
  static inline name *instance = nullptr;                                                                              \
  name();                                                                                                              \
  ~name();                                                                                                             \
                                                                                                                       \
public:                                                                                                                \
  static void Init(__VA_ARGS__);                                                                                       \
  static void Cleanup();                                                                                               \
  name(name &other) = delete;                                                                                          \
  void operator=(name const &) = delete;                                                                               \
                                                                                                                       \
private: