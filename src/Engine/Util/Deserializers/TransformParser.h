#pragma once

#include "Graphics/Transform.h"
#include "Util/Parsing.h"
#include "Util/ParsingSubfunctions.h"

namespace Engine::Util::Deserializers {
void ParseTransform(Engine::Core::Entity const &entity, char const *&data) {
  auto t = entity.AddComponent<Engine::Graphics::Transform>();

  SkipWhitespace(data);

  ENGINE_ASSERT(*data == '{', "Expected '{' after 'Transform' label!");
  data++; // Enter transform content

  char tokenBuffer[64] = {0}; // TODO: Refactor in some way so I don't have to do this in every single parser
  uint8_t lastTokenChar = 0;

  SkipWhitespace(data); // Handle case of empty transform

  while (*data != '}') {
    ReadTokenToBuffer(data, tokenBuffer, sizeof(tokenBuffer) / sizeof(uint8_t));

    // Figure out current token
    if (!strcmp("position", tokenBuffer)) {
      t->position = ParseVector3(data);
      if (*data == ',') {
        data++;
      }
    } else if (!strcmp("rotation", tokenBuffer)) {
      t->rotation = ParseQuaternion(data);
      if (*data == ',') {
        data++;
      }
    } else if (!strcmp("scale", tokenBuffer)) {
      t->scale = ParseVector3(data);
      if (*data == ',') {
        data++;
      }
    } else if (!strcmp("children", tokenBuffer)) {
      for (auto child : ParseEntityArray(data)) {
        child.GetComponent<Engine::Graphics::Transform>()->SetParent(t, false);
      }
    } else {
      ENGINE_WARNING("Unknown token '{}' in transform serialization!", tokenBuffer);
    }

    SkipWhitespace(data);
  }
  data++; // Exit transform content
}
} // namespace Engine::Util::Deserializers
