#include "Core/ECS.h"
#include "Maths/Matrix.h"

namespace Engine::Graphics
{
    ENGINE_COMPONENT_DECLARATION(Camera) {
        Maths::Matrix4 viewProjection;
        ENGINE_COMPONENT_CONSTRUCTOR(Camera) { viewProjection = Maths::Matrix4::Perspective(0.01f, 100.0f, 45.0f, 16.0f / 9.0f) * Maths::Matrix4::LookAt({ 0, 0, 5 }, { 0, 0, 0 }, { 0, 1, 0 }); }
    };
    
} // namespace Engine::Graphics
