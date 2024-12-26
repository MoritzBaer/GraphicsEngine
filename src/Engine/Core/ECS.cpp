#include "ECS.h"

#include "Debug/Logging.h"

#define GIVE_LIFE(entity) aliveAndComponentFlags[entity] = ALIVE_FLAG;

#define KILL(entity) aliveAndComponentFlags[entity] = 0;

namespace Engine::Core {

ECS::ECS() : aliveAndComponentFlags(), firstFreeEntity(0), unusedEntityIDs(), componentArrays() {}
ECS::~ECS() {
  for (_ComponentArray *array : componentArrays) {
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

void ECS::DestroyEntity(EntityId e) {
  unusedEntityIDs.push(e);
  for (int i = 0; i < componentArrays.size(); i++) {
    if (aliveAndComponentFlags[e] & (uint64_t(1) << i)) {
      componentArrays[i]->RemoveComponent(e);
    }
  }
  KILL(e)
}

std::vector<_Component *> ECS::GetComponents(EntityId e) {
  std::vector<_Component *> result;
  for (int i = 0; i < componentArrays.size(); i++) {
    if (aliveAndComponentFlags[e] & (uint64_t(1) << i)) {
      result.push_back(componentArrays[i]->GetComponent(e));
    }
  }
  return result;
}

} // namespace Engine::Core
