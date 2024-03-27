#include "ECS.h"

#include "Debug/Logging.h"

#define GIVE_LIFE(entity)                                                                                              \
  instance->aliveAndComponentFlags[entity] = ALIVE_FLAG;                                                               \
  Entities.liveEntityMap[entity] = Entities.liveEntities.size();                                                       \
  Entities.liveEntities.push_back(entity);

#define KILL(entity)                                                                                                   \
  instance->aliveAndComponentFlags[entity] = 0;                                                                        \
  Entities.liveEntityMap[Entities.liveEntities.back()] = Entities.liveEntityMap[entity];                               \
  Entities.liveEntities[entity] = Entities.liveEntities.back();                                                        \
  Entities.liveEntities.pop_back();

namespace Engine::Core {
void ECS::Init() { instance = new ECS(); }
void ECS::Cleanup() { delete instance; }

ECS::ECS() : aliveAndComponentFlags(), firstFreeEntity(), unusedEntities(), componentArrays() {}
ECS::~ECS() {
  for (_ComponentArray *array : componentArrays) {
    delete array;
  }
}

_Entity ECS::CreateEntity() {
  _Entity newEntity;
  if (!instance->unusedEntities.empty()) {
    newEntity = instance->unusedEntities.top();
    instance->unusedEntities.pop();
  } else if (instance->firstFreeEntity < MAX_ENTITY_NUMBER) {
    newEntity = instance->firstFreeEntity++;
  } else {
    ENGINE_ERROR("Tried to create a new entity when max number was reached!")
  }

  GIVE_LIFE(newEntity)

  return newEntity;
}

void ECS::DestroyEntity(_Entity e) {
  instance->unusedEntities.push(e);
  for (int i = 0; i < instance->componentArrays.size(); i++) {
    if (instance->aliveAndComponentFlags[e] & (uint64_t(1) << i)) {
      instance->componentArrays[i]->RemoveComponent(e);
    }
  }
  KILL(e)
}

std::vector<_Component *> ECS::GetComponents(_Entity e) {
  std::vector<_Component *> result;
  for (int i = 0; i < instance->componentArrays.size(); i++) {
    if (instance->aliveAndComponentFlags[e] & (uint64_t(1) << i)) {
      result.push_back(instance->componentArrays[i]->GetComponent(e));
    }
  }
  return result;
}

void Entity::Serialize(std::stringstream &targetStream) const {
  targetStream << "{ Components: [";
  auto comps = ECS::GetComponents(id);
  bool firstComp = true;
  for (auto c : ECS::GetComponents(id)) {
    if (Util::Serializable *scomp = dynamic_cast<Util::Serializable *>(c)) {
      if (!firstComp) {
        targetStream << ", ";
      } else {
        firstComp = false;
      }
      scomp->Serialize(targetStream);
    }
  }
  targetStream << "] }";
}

} // namespace Engine::Core
