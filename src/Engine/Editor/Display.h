#pragma once

#include "Core/ECS.h"
#include "Publishable.h"
#include "Util/Serializable.h"

namespace Engine::Editor {

ENGINE_COMPONENT_DECLARATION(Display), public Publishable, public Util::Serializable {
  char label[64];
  ENGINE_COMPONENT_CONSTRUCTOR(Display), Publishable("Name"), label("unnamed entity") {}
  std::vector<Publication> GetPublications() override {
    return {Publication{.label = label, .type = Publication::Type::TEXT}};
  }

  inline void AssignLabel(const char *newLabel) {
    std::fill(label, label + sizeof(label) / sizeof(char), 0);
    strcpy(label, newLabel);
  }

  inline void Serialize(std::stringstream & stream) const override { stream << "Display: { label: " << label << " }"; }
};

} // namespace Engine::Editor
