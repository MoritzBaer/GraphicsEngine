#include "Engine/Engine.h"
#include "Engine/Graphics/Shader.h"
#include <fstream>
#include "Engine/Debug/Logging.h"
#include "Engine/AssetManager.h"
#include "Core/ECS.h"
#include "Maths/Matrix.h"

ENGINE_COMPONENT_DECLARATION(TransformComponent) {
    Engine::Maths::Matrix4 locrot;

    ENGINE_COMPONENT_CONSTRUCTOR(TransformComponent) { }
};


int main() {
    Engine::Init("Test project");
    Engine::RunMainLoop();
    Engine::Cleanup();

    return 0;
}