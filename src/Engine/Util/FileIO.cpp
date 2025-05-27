#include "FileIO.h"

#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "Macros.h"
#include <filesystem>
#include <fstream>
#include <thread>

namespace Engine::Util::FileIO {
std::vector<char> ReadFile(char const *fileName) {
  PROFILE_FUNCTION()
  std::vector<char> buffer{};
  try {
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    ENGINE_ASSERT(file.is_open(), "Failed to open file {}!", fileName)

    size_t fileSize = (size_t)file.tellg();
    buffer = std::vector<char>(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

  } catch (std::ifstream::failure const &e) {
    ENGINE_ERROR("Failed to read file {}: {}", fileName, e.what())
  }

  return buffer;
}

void WriteFile(char const *fileName, char const *data) {
  PROFILE_FUNCTION()
  try {
    std::ofstream file(fileName, std::ios::binary);

    ENGINE_ASSERT(file.is_open(), "Failed to open file {}!", fileName)

    file.write(data, strlen(data));
    file.close();
  } catch (std::ofstream::failure const &e) {
    ENGINE_ERROR("Failed to write file {}: {}", fileName, e.what())
  }
}

void CopyFilesystemObject(char const *src, char const *dst, std::filesystem::copy_options options) {
  try {
    std::filesystem::copy(src, dst, options);
  } catch (std::filesystem::filesystem_error const &e) {
    ENGINE_ERROR("Failed to copy file from {} to {}: {}", src, dst, e.what())
  }
  ENGINE_ASSERT(std::filesystem::exists(dst), "Failed to copy file from {} to {}!", src, dst)
}

void CopyFile(char const *src, char const *dst, bool replaceIfNewer) {
  CopyFilesystemObject(src, dst,
                       replaceIfNewer ? std::filesystem::copy_options::overwrite_existing
                                      : std::filesystem::copy_options::update_existing);
}
void CopyDirectory(char const *src, char const *dst, bool replaceIfNewer) {
  CopyFilesystemObject(src, dst,
                       (replaceIfNewer ? std::filesystem::copy_options::overwrite_existing
                                       : std::filesystem::copy_options::update_existing) |
                           std::filesystem::copy_options::recursive);
}
} // namespace Engine::Util::FileIO
