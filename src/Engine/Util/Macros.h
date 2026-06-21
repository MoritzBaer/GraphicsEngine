#include <unistd.h>
#ifdef LOGGING_INCLUDED
#ifndef _LOGGING_MACROS_INCLUDED
#define _LOGGING_MACROS_INCLUDED

#define __FILE_NAME_LOG__ (&__FILE__[SOURCE_PATH_SIZE])

inline Engine::Debug::Logging::Logger engineLogger = Engine::Debug::Logging::Logger("Engine");

#define ENGINE_LOG_LEVEL_DEBUG 3
#define ENGINE_LOG_LEVEL_INFO 2
#define ENGINE_LOG_LEVEL_WARN 1
#define ENGINE_LOG_LEVEL_ERROR 0

#if ENGINE_LOG_LEVEL < ENGINE_LOG_LEVEL_DEBUG
#define ENGINE_DEBUG(format, ...)
#else
#define ENGINE_DEBUG(format, ...)                                                                                      \
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

#ifndef NDEBUG
#ifdef _WIN32
#include <windows.h> // IsDebuggerPresent()
#elif defined(__linux__)
#include <cstdlib>  // atoi()
#include <cstring>  // strstr()
#include <fcntl.h>  // open(), O_RDONLY
#include <unistd.h> // read(), close(), ssize_t
#elif defined(__APPLE__)
#include <sys/sysctl.h> // sysctl(), CTL_KERN, KERN_PROC, kinfo_proc, P_TRACED
#include <unistd.h>     // getpid()
#endif

inline bool __debugger_attached() {
#ifdef _WIN32
  return IsDebuggerPresent();
#elif defined(__linux__)
  // TracerPid is non-zero when a debugger is attached
  char buf[4096];
  int fd = open("/proc/self/status", O_RDONLY);
  if (fd == -1)
    return false;
  ssize_t n = read(fd, buf, sizeof(buf) - 1);
  close(fd);
  if (n <= 0)
    return false;
  buf[n] = '\0';
  const char *p = strstr(buf, "TracerPid:");
  return p && atoi(p + 10) != 0;
#elif defined(__APPLE__)
  // Use sysctl on macOS
  int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()};
  struct kinfo_proc info{};
  size_t size = sizeof(info);
  sysctl(mib, 4, &info, &size, nullptr, 0);
  return info.kp_proc.p_flag & P_TRACED;
#else
  return false;
#endif
}

#if defined(_MSC_VER)
#define __ENGINE_UNCONDITIONAL_BREAKPOINT __debugbreak();
#elif defined(__APPLE__) || defined(__linux__)
#if defined(__i386__) || defined(__x86_64__)
#define __ENGINE_UNCONDITIONAL_BREAKPOINT __asm__ volatile("int3; nop");
#elif defined(__arm__) || defined(__aarch64__)
#define __ENGINE_UNCONDITIONAL_BREAKPOINT __asm__ volatile("brk #0");
#else
#include <csignal>
#define __ENGINE_UNCONDITIONAL_BREAKPOINT raise(SIGTRAP); // Portable fallback
#endif
#else
#include <csignal>
#define __ENGINE_UNCONDITIONAL_BREAKPOINT raise(SIGTRAP);
#endif
#define __ENGINE_BREAKPOINT                                                                                            \
  if (__debugger_attached()) {                                                                                         \
    __ENGINE_UNCONDITIONAL_BREAKPOINT                                                                                  \
  }
#endif

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