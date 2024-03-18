#pragma once

#include "Core/ECS.h"
#include "Maths/Matrix.h"

namespace Engine::Graphics
{
    ENGINE_COMPONENT_DECLARATION(Transform) {
        Maths::Matrix4 modelMatrix;

        ENGINE_COMPONENT_CONSTRUCTOR(Transform), modelMatrix(Maths::Matrix4::Identity()) {}
    };
} // namespace Engine::Graphics
