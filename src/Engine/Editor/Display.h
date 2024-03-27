#pragma once

#include "Core/ECS.h"
#include "Publishable.h"

namespace Engine::Editor {

ENGINE_COMPONENT_DECLARATION(Display), public Publishable {
  const char *label;
  ENGINE_COMPONENT_CONSTRUCTOR(Display), Publishable("Name"), label("unnamed entity") {}
  std::vector<Publication> GetPublications() override {
    return {Publication{.label = label, .type = Publication::Type::TEXT}};
  }
};

} // namespace Engine::Editor
