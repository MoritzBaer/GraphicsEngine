#pragma once

#include <chrono>

namespace Engine::Time
{
    inline std::chrono::time_point<std::chrono::steady_clock> engineStart = std::chrono::steady_clock::now();
    
    inline float time = 0;
    inline float deltaTime = 0;

    inline void Update() {
        float newTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - engineStart).count() / 1000000.0f;
        
        deltaTime = newTime - time;
        time = newTime;
    }
} // namespace Engine::Time