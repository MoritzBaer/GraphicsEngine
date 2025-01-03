#include "ECS.h"

#include "Debug/Logging.h"

#define GIVE_LIFE(entity) aliveAndComponentFlags[entity] = ALIVE_FLAG;

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

Component *ECS::GetComponent(EntityId e, ComponentIndex componentIndex) {
  if (!(aliveAndComponentFlags[e] & ALIVE_FLAG)) {
    ENGINE_ERROR("Tried to query component off dead entity!") return nullptr;
  }
  if (!HasComponent(e, componentIndex)) {
    ENGINE_ERROR("Tried to query component the entity does not have!") return nullptr;
  }
  return componentArrays[componentIndex]->GetComponent(e);
}

bool ECS::HasComponent(EntityId e, ComponentIndex componentIndex) {
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

ECS::ECS() : aliveAndComponentFlags(), firstFreeEntity(0), unusedEntityIDs(), componentArrays() {}
ECS::~ECS() {
  for (ComponentArray *array : componentArrays) {
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

Entity ECS::DuplicateEntity(EntityId e) {
  EntityId newEntity = _CreateEntity();
  for (int i = 0; i < componentArrays.size(); i++) {
    if (aliveAndComponentFlags[e] & (uint64_t(1) << i)) {
      AddComponent(newEntity, i)->CopyFrom(GetComponent(e, i));
    }
  }
  ENGINE_ASSERT(aliveAndComponentFlags[e] == aliveAndComponentFlags[newEntity], "Entity duplication failed!")
  return Entity(newEntity, this);
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

std::vector<Component *> ECS::GetComponents(EntityId e) {
  std::vector<Component *> result;
  if (e == EntityId(-1) || !(aliveAndComponentFlags[e] & ALIVE_FLAG)) {
    return result;
  }
  for (int i = 0; i < componentArrays.size(); i++) {
    if (aliveAndComponentFlags[e] & (uint64_t(1) << i)) {
      result.push_back(componentArrays[i]->GetComponent(e));
    }
  }
  return result;
}

} // namespace Engine::Core
