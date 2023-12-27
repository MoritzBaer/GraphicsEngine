#include <string>
#include <vector>
#include "FileIO.h"

namespace Engine::Util
{
    struct InlinedShader;

    struct CompilerMessage {
        std::string fileName;
        uint32_t line;
        std::string message;
    };

    struct PreprocessedShader {

            PreprocessedShader(char const * file);
            char const * GetPreprocessedCode();
            uint32_t GetNumberOfLines();
            const std::string GetSourceFile();
            const CompilerMessage ReconstructMessage(CompilerMessage const& message);

        private:
            std::string preprocessedCode;
            std::string sourceFile;
            std::vector<InlinedShader> inclusions;
            uint32_t numberOfLines;

            void Preprocess(const char * file);
    };

    struct InlinedShader {
        PreprocessedShader code;
        uint32_t startAtLine;
        uint32_t endAtLine;
    };
} // namespace Engine::Util
