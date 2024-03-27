#pragma once

#include "Graphics/MeshRenderer.h"
#include "Util/Parsing.h"
#include "Util/ParsingSubfunctions.h"

namespace Engine::Util::Deserializers {
inline void ParseMeshRenderer(Core::Entity const &entity, char const *&data) {
  auto m = entity.AddComponent<Graphics::MeshRenderer>();
  PARSE_BLOCK(data,
              FIRST_TOKEN_REACTION("mesh", m->AssignMesh(ReadString(data).c_str()))
                  LATER_TOKEN_REACTION("material", m->AssignMaterial(ReadString(data).c_str())),
              "MeshRenderer")
}

} // namespace Engine::Util::Deserializers
