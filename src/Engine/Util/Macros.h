#ifdef LOGGING_INCLUDED
#ifndef _LOGGING_MACROS_INCLUDED
#define _LOGGING_MACROS_INCLUDED


#define __FILE_NAME_LOG__ (&__FILE__[SOURCE_PATH_SIZE])

#ifndef NDEBUG
#ifdef __debugbreak
#define __ENGINE_BREAKPOINT __debugbreak();
#else
#include <csignal>
#ifdef SIGTRAP
#define __ENGINE_BREAKPOINT std::raise(SIGTRAP);
#else
#define __ENGINE_BREAKPOINT std::raise(SIGABRT);
#endif
#endif
#else 
#define __ENGINE_BREAKPOINT
#endif

inline Engine::Debug::Logging::Logger engineLogger = Engine::Debug::Logging::Logger("Engine");

#define ENGINE_LOG_LEVEL_DEBUG 3
#define ENGINE_LOG_LEVEL_INFO 2
#define ENGINE_LOG_LEVEL_WARN 1
#define ENGINE_LOG_LEVEL_ERROR 0

#if ENGINE_LOG_LEVEL < ENGINE_LOG_LEVEL_DEBUG
#define ENGINE_DEBUG(format, ...)
#else 
#define ENGINE_DEBUG(format, ...)                                                                                    \
  {                                                                                                                    \
    engineLogger.PrintMessage(format " ({}({},0))", __VA_OPT__(__VA_ARGS__, ) __FILE_NAME_LOG__, __LINE__);            \
  }
#endif
#define ENGINE_MESSAGE(format, ...)                                                                                    \
  {                                                                                                                    \
    engineLogger.PrintMessage(format " ({}({},0))", __VA_OPT__(__VA_ARGS__, ) __FILE_NAME_LOG__, __LINE__);            \
  }
#define ENGINE_WARNING(format, ...)                                                                                    \
  {                                                                                                                    \
    engineLogger.PrintWarning(format " ({}({},0))", __VA_OPT__(__VA_ARGS__, ) __FILE_NAME_LOG__, __LINE__);            \
  }
#define ENGINE_SUCCESS(format, ...)                                                                                    \
  {                                                                                                                    \
    engineLogger.PrintSuccess(format " ({}({},0))", __VA_OPT__(__VA_ARGS__, ) __FILE_NAME_LOG__, __LINE__);            \
  }
#define ENGINE_ERROR(format, ...)                                                                                      \
  {                                                                                                                    \
    engineLogger.PrintError(format " ({}({},0))", __VA_OPT__(__VA_ARGS__, ) __FILE_NAME_LOG__, __LINE__);              \
    __ENGINE_BREAKPOINT;                                                                                               \
  }

#define ENGINE_ASSERT(condition, format, ...)                                                                          \
  if (!(condition)) {                                                                                                  \
    ENGINE_ERROR(format __VA_OPT__(, __VA_ARGS__))                                                                     \
  }

#include "vulkan/vk_enum_string_helper.h"
#define VULKAN_ASSERT(call, format, ...)                                                                               \
  VkResult VAR_WITH_LINE(result) = call;                                                                               \
  if (VAR_WITH_LINE(result) != VK_SUCCESS) {                                                                           \
    ENGINE_ERROR("Vulkan error: {}, message: " format,                                                                 \
                 string_VkResult(VAR_WITH_LINE(result)) __VA_OPT__(, __VA_ARGS__))                                     \
  }
#endif
#endif

#ifndef _MACROS_INCLUDED
#define _MACROS_INCLUDED

#define _CAT(a, b)                                                                                                     \
  a##b // I don't understand the preprocessor well enough to understand why this is necessary, but apparently it is...
#define CAT(a, b) _CAT(a, b)
#define VAR_WITH_LINE(name) CAT(name, __LINE__)

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

#endif