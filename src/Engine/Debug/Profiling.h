#pragma once

#include <chrono>
#include <span>
#include <vector>

#ifdef RUN_PROFILER
#define PROFILE_SCOPE(name) Engine::Debug::Profiling::LifeTimer CAT(__t, __LINE__)(name);
#else
#define PROFILE_SCOPE(name)
#endif
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCSIG__)

#ifdef RUN_PROFILER
#define BEGIN_PROFILE_SESSION() Engine::Debug::Profiling::__profiles = {};
#define WRITE_PROFILE_SESSION(name)                                                                                    \
  Engine::Debug::Profiling::ProfileWriter writer("profiles/" name ".json");                                            \
  writer.WriteSession(Engine::Debug::Profiling::__profiles);
#else
#define BEGIN_PROFILE_SESSION()
#define WRITE_PROFILE_SESSION(name)
#endif

namespace Engine::Debug::Profiling {
struct Profile;

inline std::vector<Profile> __profiles{};

struct Profile {
  const char *name;
  int64_t start;    // In milliseconds since the engine start
  int64_t duration; // In milliseconds
};

class LifeTimer {
private:
  const char *title;
  const std::chrono::time_point<std::chrono::steady_clock> birth;

public:
  LifeTimer(const char *title);
  ~LifeTimer();
};

class ProfileWriter {
  const char *outFilePath;

public:
  ProfileWriter(const char *outputPath) : outFilePath(outputPath) {}
  void WriteSession(std::span<Profile> const &profiles) const;
};
} // namespace Engine::Debug::Profiling
