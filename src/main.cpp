#include "Engine/Math/Matrix.h"

#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"

#include <iostream>

int main() {

    Engine::Math::Vector3 vector{ 0.0f, 1.0f, 2.0f };

    Engine::Math::Vector2 xz = vector.XZ();
    auto xyn = xz.Normalized();
    vector.XY() = xz.Normalized();
    vector.XY() *= 2.0f;

    return 0;
}