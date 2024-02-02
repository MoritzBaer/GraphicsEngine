#include "Engine/Engine.h"

int main() {

    Engine::Init("Test project");

    Engine::RunMainLoop();

    Engine::Cleanup();

    return 0;
}