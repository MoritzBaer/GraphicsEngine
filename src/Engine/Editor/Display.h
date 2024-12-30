#pragma once

#include <array>

#include "Core/ECS.h"
#include "Publishable.h"
#include "json-parsing.h"

namespace Engine::Editor {

ENGINE_COMPONENT_DECLARATION(Display), public Publishable {
  std::array<char, 64> label;
  ENGINE_COMPONENT_CONSTRUCTOR(Display), Publishable("Name"), label("unnamed entity") {}
  std::vector<Publication> GetPublications() override {
    return {Publication{.label = label.data(), .type = Publication::Type::TEXT}};
  }

  inline void AssignLabel(const char *newLabel) {
    std::fill(label.begin(), label.end(), 0);
    strcpy(label.data(), newLabel);
  }
};

} // namespace Engine::Editor