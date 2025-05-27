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

void CopyFile(char const *src, char const *dst, bool replaceIfNewer = false);
inline void CopyFile(string const &src, char const *dst, bool replaceIfNewer = false) { CopyFile(src.c_str(), dst, replaceIfNewer); }
inline void CopyFile(char const *src, string const &dst, bool replaceIfNewer = false) { CopyFile(src, dst.c_str(), replaceIfNewer); }
inline void CopyFile(string const &src, string const &dst, bool replaceIfNewer = false) { CopyFile(src.c_str(), dst.c_str(), replaceIfNewer); }

void CopyDirectory(char const *src, char const *dst, bool replaceIfNewer = false);
inline void CopyDirectory(string const &src, char const *dst, bool replaceIfNewer = false) { CopyDirectory(src.c_str(), dst, replaceIfNewer); }
inline void CopyDirectory(char const *src, string const &dst, bool replaceIfNewer = false) { CopyDirectory(src, dst.c_str(), replaceIfNewer); }
inline void CopyDirectory(string const &src, string const &dst, bool replaceIfNewer = false) { CopyDirectory(src.c_str(), dst.c_str()), replaceIfNewer; }
} // namespace Engine::Util::FileIO
