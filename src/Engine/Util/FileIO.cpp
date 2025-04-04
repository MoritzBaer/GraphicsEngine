#include "FileIO.h"

#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "Macros.h"
#include <filesystem>
#include <fstream>

namespace Engine::Util::FileIO {
std::vector<char> ReadFile(char const *fileName) {
  PROFILE_FUNCTION()
  std::ifstream file(fileName, std::ios::ate | std::ios::binary);

  ENGINE_ASSERT(file.is_open(), "Failed to open file {}!", fileName)

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}
void WriteFile(char const *fileName, char const *data) {
  PROFILE_FUNCTION()
  std::ofstream file(fileName, std::ios::binary);

  ENGINE_ASSERT(file.is_open(), "Failed to open file {}!", fileName)

  file.write(data, strlen(data));
  file.close();
}

void CopyFilesystemObject(char const *src, char const *dst, std::filesystem::copy_options options) {
  std::filesystem::copy(src, dst, options);
  ENGINE_ASSERT(std::filesystem::exists(dst), "Failed to copy file from {} to {}!", src, dst)
}

void CopyFile(char const *src, char const *dst) {
  CopyFilesystemObject(src, dst, std::filesystem::copy_options::overwrite_existing);
}
void CopyDirectory(char const *src, char const *dst) {
  CopyFilesystemObject(src, dst,
                       std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
}
} // namespace Engine::Util::FileIO
