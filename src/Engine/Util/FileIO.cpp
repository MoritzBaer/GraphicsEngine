#include "FileIO.h"

#include <fstream>
#include "Macros.h"
#include "Debug/Logging.h"

namespace Engine::Util::FileIO
{
    std::vector<char> ReadFile(char const * fileName) {
        std::ifstream file(fileName, std::ios::ate | std::ios::binary);
        
        ENGINE_ASSERT(file.is_open(), "Failed to open file {}!", fileName)

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }
} // namespace Engine::Util::FileIO
