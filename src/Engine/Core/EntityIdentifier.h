#pragma once

#include "ECS.h"
#include <unordered_map>

namespace Engine::Core {
struct EntityIdentifier : public ComponentT<EntityIdentifier> {
  std::string uniqueIdentifier;

  EntityIdentifier(Core::Entity entity) : ComponentT<EntityIdentifier>(entity) {}

  inline void CopyFrom(EntityIdentifier const &other) override { uniqueIdentifier = other.uniqueIdentifier; }
};

class IdentifierStorage {
  std::unordered_map<std::string, Entity> identifiedEntities;

public:
  IdentifierStorage() : identifiedEntities() {}

  inline Entity &GetEntity(std::string const &identifier) { return identifiedEntities[identifier]; }
  inline Entity &GetEntity(const char *identifier) { return GetEntity(std::string(identifier)); }

  inline void Rebuild(ECS & ecs) {
    identifiedEntities.clear();
    auto identifiers = ecs.FilterEntities<EntityIdentifier>();
    for (auto [id] : identifiers) {
        identifiedEntities.insert({id->uniqueIdentifier, id->entity});
    }
  }
};
}; // namespace Engine::Core