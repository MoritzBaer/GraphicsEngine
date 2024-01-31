#include "Engine/Maths/Matrix.h"


int main() {

    Engine::Maths::Vector3 vector{ 0.0f, 1.0f, 2.0f };

    Engine::Maths::Vector2 xz = vector.XZ();
    auto xyn = xz.Normalized();
    vector.XY() = xz.Normalized();
    vector.XY() *= 2.0f;

    return 0;
}