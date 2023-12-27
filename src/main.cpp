/*  #define TEST_SHADER 1      

    test code

    #define TEST_SHADER 2

    test code

    #define TEST_SHADER 3

    test code;

    more test code;

    more test code;
*/
#include "Engine/Util/Preprocessing.h"
#include <iostream>

void PrintMessage(Engine::Util::CompilerMessage const& message) {
    std::cout << "[MESSAGE] " << message.message << " (" << message.fileName << ":" << message.line << ")\n";
}

int main() {
    Engine::Util::PreprocessedShader shader = Engine::Util::PreprocessedShader("test.frag");
    PrintMessage(shader.ReconstructMessage({ "test.frag", 1, "Nothing wrong, actually" }));
    PrintMessage(shader.ReconstructMessage({ "test.frag", 2, "Nothing wrong, actually" }));
    PrintMessage(shader.ReconstructMessage({ "test.frag", 4, "Nothing wrong, actually" }));
    PrintMessage(shader.ReconstructMessage({ "test.frag", 5, "Nothing wrong, actually" }));
    PrintMessage(shader.ReconstructMessage({ "test.frag", 7, "Nothing wrong, actually" }));
    PrintMessage(shader.ReconstructMessage({ "test.frag", 8, "Nothing wrong, actually" }));
    PrintMessage(shader.ReconstructMessage({ "test.frag", 9, "Nothing wrong, actually" }));
    PrintMessage(shader.ReconstructMessage({ "test.frag", 10, "Nothing wrong, actually" }));
    PrintMessage(shader.ReconstructMessage({ "test.frag", 11, "Nothing wrong, actually" }));
    PrintMessage(shader.ReconstructMessage({ "test.frag", 12, "Nothing wrong, actually" }));
    PrintMessage(shader.ReconstructMessage({ "test.frag", 13, "Nothing wrong, actually" }));
    PrintMessage(shader.ReconstructMessage({ "test.frag", 14, "Nothing wrong, actually" }));
    PrintMessage(shader.ReconstructMessage({ "test.frag", 15, "Nothing wrong, actually" }));
    return 0;
}