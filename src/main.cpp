#include "Engine/Engine.h"
#include "Engine/Graphics/Shader.h"
#include <fstream>
#include "Engine/Debug/Logging.h"
#include "Engine/AssetManager.h"
#include "Core/ECS.h"
#include "Maths/Matrix.h"
#include <iostream>
#include <iomanip>

#include "glm/matrix.hpp"
#include "glm/gtx/transform.hpp"

void printForMat(void * mat) {
    for (int i = 0; i < 16; i++) { 
        float val = *(((float *)mat) + i);
        if(!std::signbit(val)) { std::cout << " "; }
        std::cout << val << " "; 
    }
}

int main() {

    Engine::Init("Test project");
    Engine::RunMainLoop();
    Engine::Cleanup();

    return 0;
}