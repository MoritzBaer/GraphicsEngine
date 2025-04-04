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

void CopyFile(char const *src, char const *dst);
inline void CopyFile(string const &src, char const *dst) { CopyFile(src.c_str(), dst); }
inline void CopyFile(char const *src, string const &dst) { CopyFile(src, dst.c_str()); }
inline void CopyFile(string const &src, string const &dst) { CopyFile(src.c_str(), dst.c_str()); }

void CopyDirectory(char const *src, char const *dst);
inline void CopyDirectory(string const &src, char const *dst) { CopyDirectory(src.c_str(), dst); }
inline void CopyDirectory(char const *src, string const &dst) { CopyDirectory(src, dst.c_str()); }
inline void CopyDirectory(string const &src, string const &dst) { CopyDirectory(src.c_str(), dst.c_str()); }
} // namespace Engine::Util::FileIO
