#pragma once

#include <string>
#include <vector>

using std::string;

namespace Engine::Util::FileIO {
std::vector<char> ReadFile(char const *fileName);
inline std::vector<char> ReadFile(string const &fileName) { return ReadFile(fileName.c_str()); }

void WriteFile(char const *fileName, char const *data);
inline void WriteFile(char const *fileName, string const &data) { WriteFile(fileName, data.c_str()); }
inline void WriteFile(string const &fileName, const char *data) { WriteFile(fileName.c_str(), data); }
inline void WriteFile(string const &fileName, string const &data) { WriteFile(fileName.c_str(), data.c_str()); }
} // namespace Engine::Util::FileIO
