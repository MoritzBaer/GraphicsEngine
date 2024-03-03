#pragma once

#include <vector>
#include <string>

namespace Engine::Util::FileIO
{
    std::vector<char> ReadFile(char const * fileName);
    inline std::vector<char> ReadFile(std::string const & fileName) { return ReadFile(fileName.c_str()); }
} // namespace Engine::Util
