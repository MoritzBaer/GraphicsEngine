#pragma once

#include "Core/ECS.h"

namespace Engine::Editor {

ENGINE_COMPONENT_DECLARATION(Display) {
  const char *label;

  ENGINE_COMPONENT_CONSTRUCTOR(Display), label("unnamed entity") {}
};

} // namespace Engine::Editor
