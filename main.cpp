#include "Math/Matrix.h"

int main() {
    float values[3] = { 0.5, 1, -2.5 };
    Engine::Math::Vector4 position{0.5f, 1.0f, 1.0f, 3.0f, 2.0f};
    Engine::Math::Vector4 normalizedPosition = position.Normalized();
    
    float delta = (normalizedPosition - (5 * position).Normalized()).Length();

    Engine::Math::MatrixT<3, 3, float> mat1{1.0f,1.0f,1.0f,1.0f,0.0f,1.0f,0.0f,1.0f,1.0f};
    Engine::Math::MatrixT<3, 2, float> mat2{1.0f,1.0f,1.0f,1.0f,1.0f,1.0f};
    auto mat3 = mat1.Inverse();
    auto mat4 = mat1 * mat3;
}