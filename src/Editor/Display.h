#pragma once

#include <array>

#include "Core/ECS.h"
#include "Debug/Logging.h"
#include "Publishable.h"
#include "json-parsing.h"

using namespace Engine;

namespace Editor {

struct Display : public Core::Component {
  std::array<char, 64> label;
  Display(Core::Entity entity) : Core::Component(entity), label("unnamed entity") {}

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

} // namespace Editor