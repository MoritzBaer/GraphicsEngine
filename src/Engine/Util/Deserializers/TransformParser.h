#pragma once

#include "Graphics/Transform.h"
#include "Util/Parsing.h"
#include "Util/ParsingSubfunctions.h"

namespace Engine::Util::Deserializers {
void ParseTransform(Core::Entity const &entity, char const *&data) {
  auto t = entity.AddComponent<Graphics::Transform>();

  PARSE_BLOCK(
      data,
      FIRST_TOKEN_REACTION("position", t->position = ParseVector3(data))
          LATER_TOKEN_REACTION("rotation", t->rotation = ParseQuaternion(data))
              LATER_TOKEN_REACTION("scale", t->scale = ParseVector3(data)) LATER_TOKEN_REACTION(
                  "children",
                  for (auto child
                       : ParseEntityArray(data)) { child.GetComponent<Graphics::Transform>()->SetParent(t, false); }),
      "Transform")
}

} // namespace Engine::Util::Deserializers
