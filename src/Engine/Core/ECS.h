#pragma once

#include "Util/Macros.h"

#include <array>
#include <inttypes.h>
#include <stack>
#include <vector>

#define MAX_COMPONENT_NUMBER 60
#define MAX_ENTITY_NUMBER (1 << 16)
#define ALIVE_FLAG (uint64_t(1) << 63)
#define ACTIVE_FLAG (uint64_t(1) << 62)
#define COMPONENT_FLAG(ComponentType) (uint64_t(1) << ComponentID<ComponentType>::value)

namespace Engine::Core {
using EntityId = uint16_t;
using componentID = uint8_t;
using ComponentIndex = uint16_t; // Should be the same size as EntityId

template <typename T> struct ComponentID {
  inline static componentID value = componentID(-1);
};

class Component;
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
    virtual ComponentArray *InitEmptyForOtherECS(ECS *otherECS) const = 0;
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
    inline ComponentArray *InitEmptyForOtherECS(ECS *otherECS) const override {
      return new ComponentArrayT<C>(otherECS);
    }
  };

  std::vector<uint64_t> aliveAndComponentFlags;
  EntityId firstFreeEntity;
  std::stack<EntityId> unusedEntityIDs;
  std::array<ComponentArray *, MAX_COMPONENT_NUMBER> componentArrays;
  inline static componentID nextComponentID = 0;

  Component *AddComponent(EntityId e, ComponentIndex componentIndex);
  Component *GetComponent(EntityId e, ComponentIndex componentIndex) const;
  bool HasComponent(EntityId e, ComponentIndex componentIndex) const;
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

  template <class C> static void RegisterComponent(); // Components can only be registered after initialization

  template <class C> inline C *AddComponent(EntityId e);
  template <class C> inline C *GetComponent(EntityId e) const;
  template <class C> inline bool HasComponent(EntityId e) const;
  template <class C> inline void RemoveComponent(EntityId e);
  inline void SetActive(EntityId e, bool active);
  inline bool IsActive(EntityId e) const;
  inline bool IsAlive(EntityId e) const;

  std::vector<Component *> GetComponents(EntityId e) const;

  template <class... Cs> std::vector<std::tuple<Cs *...>> FilterEntities(bool onlyActive = true);

  class EntityIterator;
  friend class EntityIterator;

  EntityIterator begin();
  EntityIterator end();

  Entity CopyFromOtherECS(EntityId e, ECS const *otherECS);
  void Copy(ECS const *otherECS);
};

class Entity { // Wrapper for internal entity, convenience only
  EntityId id;
  friend class ECS;
  friend class HierarchyComponent;
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
  inline bool IsAlive() const { return parentECS && parentECS->IsAlive(id); }
  inline void SetActive(bool active = true) const { parentECS->SetActive(id, active); }
  inline bool IsActive() const { return parentECS->IsActive(id); }

  inline bool operator==(Entity const &other) const { return id == other.id && parentECS == other.parentECS; }

  inline Entity Duplicate() const { return parentECS->DuplicateEntity(id); }
  inline Entity CopyToOtherECS(ECS *otherECS) const { return otherECS->CopyFromOtherECS(id, parentECS); }
  inline Entity InOtherECS(ECS *otherECS) const { return Entity(id, otherECS); }
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
  inline Component(Entity entity) : entity(entity) {}
  virtual inline ~Component() {}
  virtual inline void Update() {};
  virtual void CopyFrom(Component const *other) = 0;
};

#define TO_STR(x) #x

void _CopyError(const char *typeName);

template <class C> struct ComponentT : public Component {
  ComponentT(Entity e) : Component(e) {}
  virtual void CopyFrom(C const &other) = 0;
  void CopyFrom(Component const *other) override {
    if (auto otherComponent = dynamic_cast<C const *>(other)) {
      CopyFrom(*otherComponent);
    } else {
      _CopyError(TO_STR(C));
    }
  }
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
  C *newComponent = new C(Entity(e, parent));
  components.push_back(newComponent);
  return components.back();
}

template <class C> inline void ECS::ComponentArrayT<C>::RemoveComponent(EntityId e) {
  C *component = components[entityComponentIndexMap[e]];
  delete component;
  components[entityComponentIndexMap[e]] = components.back();
  entityComponentIndexMap[components.back()->entity.id] = entityComponentIndexMap[e];
  components.pop_back();
}

template <class C> inline void ECS::RegisterComponent() {
  if (ComponentID<C>::value == componentID(-1)) {
    ComponentID<C>::value = nextComponentID++;
  }
}

template <class C> inline C *ECS::AddComponent(EntityId e) {
  if (!componentArrays[ComponentID<C>::value]) {
    componentArrays[ComponentID<C>::value] = new ComponentArrayT<C>(this);
  }
  return dynamic_cast<C *>(AddComponent(e, ComponentID<C>::value));
}

template <class C> inline C *ECS::GetComponent(EntityId e) const {
  return dynamic_cast<C *>(GetComponent(e, ComponentID<C>::value));
}

template <class C> inline bool ECS::HasComponent(EntityId e) const {
  if (!componentArrays[ComponentID<C>::value]) {
    return false;
  }
  return HasComponent(e, ComponentID<C>::value);
}

template <class C> inline void ECS::RemoveComponent(EntityId e) { RemoveComponent(e, ComponentID<C>::value); }

template <class C> inline uint64_t get_flag() { return COMPONENT_FLAG(C); }

template <class... Cs> inline std::vector<std::tuple<Cs *...>> ECS::FilterEntities(bool onlyActive) {
  std::vector<std::tuple<Cs *...>> result;
  uint64_t filterMask = ALIVE_FLAG | (get_flag<Cs>() | ...);

  for (int e = 0; e < firstFreeEntity; e++) {
    if ((aliveAndComponentFlags[e] & filterMask) == filterMask &&
        (!onlyActive || aliveAndComponentFlags[e] & ACTIVE_FLAG)) {
      result.push_back(std::make_tuple(GetComponent<Cs>(e)...));
    }
  }

  return result;
}

inline void ECS::SetActive(EntityId e, bool active) {
  aliveAndComponentFlags[e] =
      active ? aliveAndComponentFlags[e] | ACTIVE_FLAG : aliveAndComponentFlags[e] & ~ACTIVE_FLAG;
}

inline bool ECS::IsActive(EntityId e) const { return aliveAndComponentFlags[e] & ACTIVE_FLAG; }

inline bool ECS::IsAlive(EntityId e) const { return e != EntityId(-1) && (aliveAndComponentFlags[e] & ALIVE_FLAG); }

} // namespace Engine::Core
