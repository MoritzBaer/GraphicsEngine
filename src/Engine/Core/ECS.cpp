#include "ECS.h"

#include "Core/HierarchyComponent.h"
#include "Debug/Logging.h"

#define GIVE_LIFE(entity) aliveAndComponentFlags[entity] = ALIVE_FLAG | ACTIVE_FLAG;

#define KILL(entity) aliveAndComponentFlags[entity] = 0;

namespace Engine::Core {
Component *ECS::AddComponent(EntityId e, ComponentIndex componentIndex) {
  if (!(aliveAndComponentFlags[e] & ALIVE_FLAG)) {
    ENGINE_ERROR("Tried to attach component to dead entity!") return nullptr;
  }
  if (HasComponent(e, componentIndex)) {
    ENGINE_ERROR("Tried to attach same component twice!") return nullptr;
  }
  aliveAndComponentFlags[e] |= uint64_t(1) << componentIndex;
  return componentArrays[componentIndex]->AddComponent(e);
}

Component *ECS::GetComponent(EntityId e, ComponentIndex componentIndex) const {
  if (!(aliveAndComponentFlags[e] & ALIVE_FLAG)) {
    ENGINE_ERROR("Tried to query component off dead entity!") return nullptr;
  }
  if (!HasComponent(e, componentIndex)) {
    ENGINE_ERROR("Tried to query component the entity does not have!") return nullptr;
  }
  return componentArrays[componentIndex]->GetComponent(e);
}

bool ECS::HasComponent(EntityId e, ComponentIndex componentIndex) const {
  return aliveAndComponentFlags[e] & (uint64_t(1) << componentIndex);
}

void ECS::RemoveComponent(EntityId e, ComponentIndex componentIndex) {
  if (!(aliveAndComponentFlags[e] & ALIVE_FLAG)) {
    ENGINE_ERROR("Tried to query component off dead entity!");
  }
  if (!HasComponent(e, componentIndex)) {
    ENGINE_ERROR("Tried to query component the entity does not have!");
  }
  componentArrays[componentIndex]->RemoveComponent(e);
}

ECS::ECS() : aliveAndComponentFlags(MAX_ENTITY_NUMBER), firstFreeEntity(0), unusedEntityIDs(), componentArrays() {
  componentArrays.fill(nullptr);
}
ECS::~ECS() {
  for (ComponentArray *array : componentArrays) {
    if (array)
      delete array;
  }
}

EntityId ECS::_CreateEntity() {
  EntityId newEntity;
  if (!unusedEntityIDs.empty()) {
    newEntity = unusedEntityIDs.top();
    unusedEntityIDs.pop();
  } else if (firstFreeEntity < MAX_ENTITY_NUMBER) {
    newEntity = firstFreeEntity++;
  } else {
    ENGINE_ERROR("Tried to create a new entity when max number was reached!")
  }

  GIVE_LIFE(newEntity)

  return newEntity;
}

Entity ECS::DuplicateEntity(EntityId e) { return CopyFromOtherECS(e, this); }

Entity ECS::CopyFromOtherECS(EntityId e, ECS const *otherECS) {
  EntityId newEntity = _CreateEntity();
  for (int c = 0; c < componentArrays.size(); c++) {
    if (otherECS->aliveAndComponentFlags[e] & (uint64_t(1) << c) && otherECS->componentArrays[c]) {
      if (!componentArrays[c]) {
        componentArrays[c] = otherECS->componentArrays[c]->InitEmptyForOtherECS(this);
      }
      AddComponent(newEntity, c)->CopyFrom(otherECS->GetComponent(e, c));
    }
  }
  ENGINE_ASSERT(otherECS->aliveAndComponentFlags[e] == aliveAndComponentFlags[newEntity], "Entity duplication failed!")
  return Entity(newEntity, this);
}

void ECS::Copy(ECS const *otherECS) {
  EntityId current = 0;
  while (current < otherECS->firstFreeEntity) {
    while (!(otherECS->aliveAndComponentFlags[current] & ALIVE_FLAG) && ++current < otherECS->firstFreeEntity)
      ;
    if (HasComponent(current, ComponentID<HierarchyComponent>::value) &&
        dynamic_cast<HierarchyComponent *>(otherECS->GetComponent(current, ComponentID<HierarchyComponent>::value))
            ->parent) {
      current++;
      continue;
    }
    CopyFromOtherECS(current++, otherECS);
  }
}

void ECS::DestroyEntity(EntityId e) {
  unusedEntityIDs.push(e);
  for (int i = 0; i < componentArrays.size(); i++) {
    if (aliveAndComponentFlags[e] & (uint64_t(1) << i)) {
      componentArrays[i]->RemoveComponent(e);
    }
  }
  KILL(e)
}

void _CopyError(const char * typeName) {
  ENGINE_ERROR("Tried to copy {} from different type!", typeName);
}

std::vector<Component *> ECS::GetComponents(EntityId e) const {
  std::vector<Component *> result;
  if (e == EntityId(-1) || !(aliveAndComponentFlags[e] & ALIVE_FLAG)) {
    return result;
  }
  for (int i = 0; i < componentArrays.size(); i++) {
    if (aliveAndComponentFlags[e] & (uint64_t(1) << i) && componentArrays[i]) {
      result.push_back(componentArrays[i]->GetComponent(e));
    }
  }
  return result;
}

} // namespace Engine::Core
