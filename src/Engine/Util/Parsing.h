#pragma once

#include "Core/ECS.h"
#include "Graphics/Mesh.h"
#include "Maths/Quaternion.h"

namespace Engine::Util {

Graphics::Mesh ParseOBJ(char const *charStream);

Core::Entity ParseEntity(char const *&charStream);

std::vector<Core::Entity> ParseEntityArray(char const *&charStream);

std::vector<float> ParseFloatArray(char const *&charStream);

Maths::Vector3 ParseVector3(char const *&charStream);

Maths::Quaternion ParseQuaternion(char const *&charStream);

// TODO: If performance becomes an issue, replace function pointer with ComponentDeserializer interface.
void RegisterComponentParser(const char *identifier, std::function<void(Core::Entity const &, char const *&)> parser);

} // namespace Engine::Util
