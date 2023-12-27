#include <string>

namespace Engine::Util
{
    class FileIO {
        // TODO: prevent constructing an instance
            static uint8_t callNumber;
        public:
            static std::string ReadFile(const char* filePath);
    };
} // namespace Engine::Util
