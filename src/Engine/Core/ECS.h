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

#define ENGINE_COMPONENT_DECLARATION(name) struct name : public Engine::Core::Component<name>
#define ENGINE_COMPONENT_CONSTRUCTOR(name, ...)                                                                        \
  name(Engine::Core::_Entity e, __VA_ARGS__) : Engine::Core::Component<name>(e)

#define ENGINE_NEW_ENTITY() Engine::Core::Entity(Engine::Core::ECS::CreateEntity())

namespace Engine::Core {
using _Entity = uint16_t;
using ComponentIndex = uint16_t; // Should be the same size as _Entity

struct _Component;

class Entity;
class ECS {
private:
  _SINGLETON(ECS)

  class _ComponentArray {
  protected:
    std::array<ComponentIndex, MAX_ENTITY_NUMBER> entityComponentIndexMap;

  public:
    virtual _Component *GetComponent(_Entity e) = 0;
    virtual _Component *AddComponent(_Entity e) = 0;
    virtual void RemoveComponent(_Entity e) = 0;
    virtual ~_ComponentArray() {}
  };

  template <class C> class ComponentArray : public _ComponentArray {
    std::vector<C *> components;

  public:
    ~ComponentArray();
    C *GetComponent(_Entity e);
    C *AddComponent(_Entity e);
    void RemoveComponent(_Entity e);
  };

  std::array<uint64_t, MAX_ENTITY_NUMBER> aliveAndComponentFlags;
  _Entity firstFreeEntity;
  std::stack<_Entity> unusedEntities;
  std::vector<_ComponentArray *> componentArrays;

public:
  static _Entity CreateEntity();
  static void DestroyEntity(_Entity e);

  template <class C> static void RegisterComponent(); // Components can only be registered after initialization

  template <class C> static C *AddComponent(_Entity e);
  template <class C> static C *GetComponent(_Entity e);
  template <class C> static bool HasComponent(_Entity e);
  template <class C> static void RemoveComponent(_Entity e);

  template <class... Cs> static std::vector<std::tuple<Cs *...>> FilterEntities();

  class EntityIterator;

  inline static class EntityContainer {
    friend class ECS;
    std::array<_Entity, MAX_ENTITY_NUMBER> liveEntityMap;
    std::vector<_Entity> liveEntities;

  public:
    EntityIterator begin();
    EntityIterator end();
  } Entities;
};

class Entity { // Wrapper for internal entity, convenience only
  _Entity id;

public:
  inline Entity(_Entity const &e) : id(e) {}
  inline Entity() : id(-1) {}

  template <class C> inline C *AddComponent() const { return ECS::AddComponent<C>(id); }
  template <class C> inline C *GetComponent() const { return ECS::GetComponent<C>(id); }
  template <class C> inline bool HasComponent() const { return ECS::HasComponent<C>(id); }
  template <class C> inline void RemoveComponent() const { ECS::RemoveComponent<C>(id); }
};

class ECS::EntityIterator {
  std::vector<_Entity>::iterator internalIt;

public:
  inline Entity const &operator*() const { return Entity(*internalIt); }
  inline EntityIterator &operator++() {
    internalIt++;
    return *this;
  }

  inline bool operator==(EntityIterator const &other) const { return internalIt == other.internalIt; }
  inline bool operator!=(EntityIterator const &other) const { return internalIt != other.internalIt; }

  EntityIterator(std::vector<_Entity>::iterator parent) : internalIt(parent) {}
};

struct _Component {
  Entity entity;
  inline _Component(_Entity e) : entity(e) {}
  inline ~_Component() {}
  virtual void Update(_Entity e){};
};

template <class C> struct Component : public _Component {
  static inline uint8_t COMPONENT_INDEX = -1;
  Component(_Entity e) : _Component(e) {}
};

// IMPLEMENTATIONS

inline ECS::EntityIterator ECS::EntityContainer::begin() { return EntityIterator(liveEntities.begin()); }
inline ECS::EntityIterator ECS::EntityContainer::end() { return EntityIterator(liveEntities.end()); }

template <class C> inline ECS::ComponentArray<C>::~ComponentArray() {
  for (auto c : components) {
    delete c;
  }
}

template <class C> inline C *ECS::ComponentArray<C>::GetComponent(_Entity e) {
  return components[entityComponentIndexMap[e]];
}

template <class C> inline C *ECS::ComponentArray<C>::AddComponent(_Entity e) {
  entityComponentIndexMap[e] = components.size();
  C *newComponent = new C(e);
  components.push_back(newComponent);
  return components.back();
}

template <class C> inline void ECS::ComponentArray<C>::RemoveComponent(_Entity e) {
  C *component = components[entityComponentIndexMap[e]];
  delete component;
  components[entityComponentIndexMap[e]] = components.back();
  components.pop_back();
}

template <class C> inline void ECS::RegisterComponent() {
  C::COMPONENT_INDEX = instance->componentArrays.size();
  instance->componentArrays.push_back(new ComponentArray<C>());
}

template <class C> inline C *ECS::AddComponent(_Entity e) {
  if (!(instance->aliveAndComponentFlags[e] & ALIVE_FLAG)) {
    ENGINE_ERROR("Tried to attach component to dead entity!") return nullptr;
  }
  if (HasComponent<C>(e)) {
    ENGINE_ERROR("Tried to attach same component twice!") return nullptr;
  }
  instance->aliveAndComponentFlags[e] |= COMPONENT_FLAG(C);
  return reinterpret_cast<C *>(instance->componentArrays[C::COMPONENT_INDEX]->AddComponent(e));
}

template <class C> inline C *ECS::GetComponent(_Entity e) {
  if (!(instance->aliveAndComponentFlags[e] & ALIVE_FLAG)) {
    ENGINE_ERROR("Tried to query component off dead entity!") return nullptr;
  }
  if (!HasComponent<C>(e)) {
    ENGINE_ERROR("Tried to query component the entity does not have!") return nullptr;
  }
  return reinterpret_cast<C *>(instance->componentArrays[C::COMPONENT_INDEX]->GetComponent(e));
}

template <class C> inline bool ECS::HasComponent(_Entity e) {
  return instance->aliveAndComponentFlags[e] & (COMPONENT_FLAG(C));
}

template <class C> inline void ECS::RemoveComponent(_Entity e) {
  if (!(instance->aliveAndComponentFlags[e] & ALIVE_FLAG)) {
    ENGINE_ERROR("Tried to query component off dead entity!") return nullptr;
  }
  if (!HasComponent<C>(e)) {
    ENGINE_ERROR("Tried to query component the entity does not have!") return nullptr;
  }
  instance->componentArrays[C::COMPONENT_INDEX]->RemoveComponent(e);
}

template <class C> inline uint64_t get_flag() { return COMPONENT_FLAG(C); }

template <class... Cs> inline std::vector<std::tuple<Cs *...>> ECS::FilterEntities() {
  std::vector<std::tuple<Cs *...>> result;
  uint64_t filterMask = ALIVE_FLAG | (get_flag<Cs>() | ...);

  for (int e = 0; e < MAX_ENTITY_NUMBER; e++) {
    if ((instance->aliveAndComponentFlags[e] & filterMask) == filterMask) {
      result.push_back(std::make_tuple(GetComponent<Cs>(e)...));
    }
  }

  return result;
}

} // namespace Engine::Core
