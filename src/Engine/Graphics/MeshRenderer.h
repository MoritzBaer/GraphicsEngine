#pragma once

#include "Core/ECS.h"
#include "Mesh.h"
#include "Material.h"
#include "Transform.h"

namespace Engine::Graphics
{
    ENGINE_COMPONENT_DECLARATION(MeshRenderer) {
        Mesh mesh;
        Material * material;

        ENGINE_COMPONENT_CONSTRUCTOR(MeshRenderer) { if (!entity.HasComponent<Transform>()) { entity.AddComponent<Transform>(); } }
    };
} // namespace Engine::Graphics
