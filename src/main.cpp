#include "Engine/Engine.h"
#include "Engine/Graphics/Shader.h"
#include <fstream>
#include "Engine/Debug/Logging.h"
#include "Engine/AssetManager.h"

int main() {

    Engine::Init("Test project");
    Engine::RunMainLoop();
    Engine::Cleanup();

    return 0;
}