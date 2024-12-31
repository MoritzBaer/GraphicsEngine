#pragma once

#include "Debug/Logging.h"
#include "Util/Macros.h"

#include <array>
#include <inttypes.h>
#include <stack>
#include <vector>

#define MAX_COMPONENT_NUMBER 63
#define MAX_ENTITY_NUMBER (1 << 16)
#define ALIVE_FLAG (uint64_t(1) << 63)
#define COMPONENT_FLAG(ComponentType) (uint64_t(1) << ComponentType::COMPONENT_INDEX)

#define ENGINE_COMPONENT_DECLARATION(name) struct name : public Engine::Core::ComponentT<name>
#define ENGINE_COMPONENT_CONSTRUCTOR(name, ...)                                                                        \
  name(Engine::Core::EntityId e, Engine::Core::ECS *parent, __VA_ARGS__) : Engine::Core::ComponentT<name>(e, parent)

#define ENGINE_NEW_ENTITY() Engine::Core::Entity(Engine::Core::ECS::CreateEntity())

namespace Engine::Core {
using EntityId = uint16_t;
using ComponentIndex = uint16_t; // Should be the same size as EntityId

struct Component;
class Entity;

class ECS {
private:
  class ComponentArray {
  protected:
    std::array<ComponentIndex, MAX_ENTITY_NUMBER> entityComponentIndexMap;
    ECS *parent;

  public:
    inline ComponentArray(ECS *parent) : parent(parent) {}
    virtual Component *GetComponent(EntityId e) = 0;
    virtual Component *AddComponent(EntityId e) = 0;
    virtual void RemoveComponent(EntityId e) = 0;
    virtual ~ComponentArray() {}
  };

  template <class C> class ComponentArrayT : public ComponentArray {
    std::vector<C *> components;

  public:
    inline ComponentArrayT(ECS *parent) : ComponentArray(parent) {}
    ~ComponentArrayT();
    C *GetComponent(EntityId e);
    C *AddComponent(EntityId e);
    void RemoveComponent(EntityId e);
  };

  std::array<uint64_t, MAX_ENTITY_NUMBER> aliveAndComponentFlags;
  EntityId firstFreeEntity;
  std::stack<EntityId> unusedEntityIDs;
  std::vector<ComponentArray *> componentArrays;

  Component *AddComponent(EntityId e, ComponentIndex componentIndex);
  Component *GetComponent(EntityId e, ComponentIndex componentIndex);
  bool HasComponent(EntityId e, ComponentIndex componentIndex);
  void RemoveComponent(EntityId e, ComponentIndex componentIndex);

public:
  ECS();
  ~ECS();

  EntityId _CreateEntity();
  inline Entity CreateEntity();
  Entity DuplicateEntity(EntityId e);
  inline Entity DuplicateEntity(Entity e);
  void DestroyEntity(EntityId e);
  inline void DestroyEntity(Entity e);

  template <class C> void RegisterComponent(); // Components can only be registered after initialization

  template <class C> C *AddComponent(EntityId e);
  template <class C> C *GetComponent(EntityId e);
  template <class C> bool HasComponent(EntityId e);
  template <class C> void RemoveComponent(EntityId e);

  std::vector<Component *> GetComponents(EntityId e);

  template <class... Cs> std::vector<std::tuple<Cs *...>> FilterEntities();

  class EntityIterator;
  friend class EntityIterator;

  EntityIterator begin();
  EntityIterator end();
};

class Entity { // Wrapper for internal entity, convenience only
  EntityId id;
  friend class ECS;
  ECS *parentECS;

public:
  inline Entity(EntityId const &e, ECS *parent) : id(e), parentECS(parent) {}
  inline Entity() : id(-1), parentECS(nullptr) {};

  template <class C> inline C *AddComponent() const { return parentECS->AddComponent<C>(id); }
  template <class C> inline C *GetComponent() const { return parentECS->GetComponent<C>(id); }
  template <class C> inline bool HasComponent() const { return parentECS->HasComponent<C>(id); }
  template <class C> inline void RemoveComponent() const { parentECS->RemoveComponent<C>(id); }

  inline std::vector<Component *> GetComponents() const { return parentECS->GetComponents(id); }
  inline void Destroy() const { parentECS->DestroyEntity(id); }
  inline Entity Duplicate() const { return parentECS->DuplicateEntity(id); }

  inline bool operator==(Entity const &other) const { return id == other.id && parentECS == other.parentECS; }
};

class ECS::EntityIterator {
  EntityId currentEntity = 0;
  ECS *ecs;

public:
  inline Entity const &operator*() const { return Entity(currentEntity, ecs); }
  inline EntityIterator &operator++() {
    while (!(ecs->aliveAndComponentFlags[++currentEntity] & ALIVE_FLAG) && currentEntity < ecs->firstFreeEntity)
      ;
    return *this;
  }

  inline bool operator==(EntityIterator const &other) const { return currentEntity == other.currentEntity; }
  inline bool operator!=(EntityIterator const &other) const { return currentEntity != other.currentEntity; }

  EntityIterator(EntityId e, ECS *ecs) : currentEntity(e), ecs(ecs) {}
};

class Component {
public:
  Entity entity;
  inline Component(EntityId e, ECS *ecs) : entity(e, ecs) {}
  virtual inline ~Component() {}
  virtual inline void Update() {};
  virtual void CopyFrom(Component const *other) = 0;
};

template <class C> struct ComponentT : public Component {
  static inline uint8_t COMPONENT_INDEX = -1;
  ComponentT(EntityId e, ECS *ecs) : Component(e, ecs) {}
};

// IMPLEMENTATIONS

inline Entity ECS::CreateEntity() { return Entity(_CreateEntity(), this); }
inline Entity ECS::DuplicateEntity(Entity e) { return DuplicateEntity(e.id); }
inline void ECS::DestroyEntity(Entity e) { DestroyEntity(e.id); }

inline ECS::EntityIterator ECS::begin() { return EntityIterator(0, this); }
inline ECS::EntityIterator ECS::end() { return EntityIterator(firstFreeEntity, this); }

template <class C> inline ECS::ComponentArrayT<C>::~ComponentArrayT() {
  for (auto c : components) {
    delete c;
  }
}

template <class C> inline C *ECS::ComponentArrayT<C>::GetComponent(EntityId e) {
  return components[entityComponentIndexMap[e]];
}

template <class C> inline C *ECS::ComponentArrayT<C>::AddComponent(EntityId e) {
  entityComponentIndexMap[e] = components.size();
  C *newComponent = new C(e, parent);
  components.push_back(newComponent);
  return components.back();
}

template <class C> inline void ECS::ComponentArrayT<C>::RemoveComponent(EntityId e) {
  C *component = components[entityComponentIndexMap[e]];
  delete component;
  components[entityComponentIndexMap[e]] = components.back();
  components.pop_back();
}

template <class C> inline void ECS::RegisterComponent() {
  C::COMPONENT_INDEX = componentArrays.size();
  componentArrays.push_back(new ComponentArrayT<C>(this));
}

template <class C> inline C *ECS::AddComponent(EntityId e) {
  return reinterpret_cast<C *>(AddComponent(e, C::COMPONENT_INDEX));
}

template <class C> inline C *ECS::GetComponent(EntityId e) {
  return reinterpret_cast<C *>(GetComponent(e, C::COMPONENT_INDEX));
}

template <class C> inline bool ECS::HasComponent(EntityId e) { return HasComponent(e, C::COMPONENT_INDEX); }

template <class C> inline void ECS::RemoveComponent(EntityId e) { RemoveComponent(e, C::COMPONENT_INDEX); }

template <class C> inline uint64_t get_flag() { return COMPONENT_FLAG(C); }

template <class... Cs> inline std::vector<std::tuple<Cs *...>> ECS::FilterEntities() {
  std::vector<std::tuple<Cs *...>> result;
  uint64_t filterMask = ALIVE_FLAG | (get_flag<Cs>() | ...);

  for (int e = 0; e < MAX_ENTITY_NUMBER; e++) {
    if ((aliveAndComponentFlags[e] & filterMask) == filterMask) {
      result.push_back(std::make_tuple(GetComponent<Cs>(e)...));
    }
  }

  return result;
}

} // namespace Engine::Core
