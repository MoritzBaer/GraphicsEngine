#include "Math/Matrix.h"

int main() {
    float values[3] = { 0.5, 1, -2.5 };
    Engine::Math::Vector4 position{0.5f, 1.0f, 1.0f, 3.0f, 2.0f};
    Engine::Math::Vector4 normalizedPosition = position.Normalized();
    
    float delta = (normalizedPosition - (5 * position).Normalized()).Length();

    float x = position.X() = 2.0f;
    float y = position[1];
    Engine::Math::Vector2 xy = position.XY();

    normalizedPosition.XY() = xy;

    Engine::Math::MatrixT<3, 3, float> mat1{1.0f,1.0f,1.0f,1.0f,0.0f,1.0f,0.0f,1.0f,1.0f};
    Engine::Math::MatrixT<3, 2, float> mat2{1.0f,1.0f,1.0f,1.0f,1.0f,1.0f};
    auto mat3 = mat1.Inverse();
    auto mat4 = mat1 * mat3;
}