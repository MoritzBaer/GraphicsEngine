#pragma once

#include <array>

#include "Core/ECS.h"
#include "Debug/Logging.h"
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

  inline void CopyFrom(Core::Component const *other) override {
    if (auto otherDisplay = dynamic_cast<Display const *>(other)) {
      AssignLabel(otherDisplay->label.data());
    } else {
      ENGINE_ERROR("Tried to copy Display from different type!");
    }
  }
};

} // namespace Engine::Editor