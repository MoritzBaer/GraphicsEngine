#pragma once

#include "Editor/Display.h"
#include "Util/Parsing.h"
#include "Util/ParsingSubfunctions.h"

namespace Engine::Util::Deserializers {
void ParseDisplay(Core::Entity const &entity, char const *&data) {
  auto d = entity.AddComponent<Editor::Display>();
  PARSE_BLOCK(data, FIRST_TOKEN_REACTION("label", ReadValueToBuffer(data, d->label, sizeof(d->label) / sizeof(char))),
              "Display")
}
} // namespace Engine::Util::Deserializers
