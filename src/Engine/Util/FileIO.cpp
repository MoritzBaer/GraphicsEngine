#include "FileIO.h"

uint8_t Engine::Util::FileIO::callNumber = 0;

std::string Engine::Util::FileIO::ReadFile(const char *filePath)    // TODO: implement
{
    callNumber++;
    if(callNumber == 1) {
        return std::string("#define TEST_SHADER 1\n\ntest code\n\n#include \"test_file.vert\"\n\nmore test code;");
    }
    if(callNumber == 2) {
        return std::string("#define TEST_SHADER 2\n\ntest code\n\n#include \"test_file.geom\"\n\nmore test code;");
    }
    if(callNumber == 3) {
        return std::string("#define TEST_SHADER 3\n\ntest code;");
    }
    return std::string();
}