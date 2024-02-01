#include "Engine/Engine.h"

int main() {

    Engine::Init();

    Engine::RunMainLoop();

    Engine::Cleanup();
    
    return 0;
}