#include "Profiling.h"

#include "Logging.h"

#include <fstream>

#define JSON_PARAM(name, value) << "    \"" name "\": \"" << value << "\""

std::chrono::time_point<std::chrono::steady_clock> _init_time = std::chrono::steady_clock::now();

Engine::Debug::Profiling::LifeTimer::LifeTimer(const char * title) : title(title), birth(std::chrono::steady_clock::now()) { }

Engine::Debug::Profiling::LifeTimer::~LifeTimer() { 
    int64_t lifetime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - birth).count();
    __profiles.push_back(Profile {
        .name = title,
        .start = std::chrono::duration_cast<std::chrono::microseconds>(birth - _init_time).count(),
        .duration = lifetime
    });
}

void WriteProfile(Engine::Debug::Profiling::Profile const & profile, std::ofstream & targetStream) {
    targetStream << "  {\n"
        JSON_PARAM("name", profile.name)        << ",\n"
        JSON_PARAM("cat",  "")                  << ",\n"
        JSON_PARAM("ph",   "X")                 << ",\n"
        JSON_PARAM("ts",   profile.start)       << ",\n"
        JSON_PARAM("dur",  profile.duration)    << ",\n"
        JSON_PARAM("pid",  "0")                 << ",\n"
        JSON_PARAM("tid",  "0")                 << ",\n"
        JSON_PARAM("args", "{}")
        << "  }";
        
}

void Engine::Debug::Profiling::ProfileWriter::WriteSession(std::span<Profile> const &profiles) const
{
    if (profiles.empty()) { return; }
    std::ofstream out(outFilePath);
    out << "[\n";
    for(int i = 0; i < profiles.size() - 1; i++) { 
        WriteProfile(profiles[i], out);
        out << ",\n"; 
    }
    WriteProfile(profiles.back(), out);
    out << "\n]";
    out.close();
}
