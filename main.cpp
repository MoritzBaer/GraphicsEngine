#include "Math/Vector.h"

int main() {
    float values[3] = { 0.5, 1, -2.5 };
    Engine::Math::Vector4 position{0.5f, 1.0f, 1.0f, 3.0f, 2.0f};
    Engine::Math::Vector4 normalizedPosition = position.Normalized();
    
    float delta = (normalizedPosition - (5 * position).Normalized()).Length();
}