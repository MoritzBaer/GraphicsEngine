#pragma once

#include <chrono>

namespace Engine::Core {
struct Clock {
private:
  std::chrono::time_point<std::chrono::steady_clock> engineStart;

public:
  float time = 0;
  float deltaTime = 0;

  inline void Start() { engineStart = std::chrono::steady_clock::now(); }

  inline void Update() {
    float newTime =
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - engineStart).count() /
        1000000.0f;

    deltaTime = newTime - time;
    time = newTime;
  }
};
} // namespace Engine::Core