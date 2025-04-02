#pragma once

#include "Core/ECS.h"
#include "Core/Time.h"

namespace Engine::Core {

struct ScriptComponent;

class Script {
protected:
  Entity entity;

public:
  bool started;
  Script(Entity entity) : entity(entity), started(false) {}
  virtual ~Script() = default;
  virtual void OnCreate() {}
  virtual void OnDestroy() {}
  virtual void OnStart() {}
  virtual void OnUpdate(Clock const &clock) {}
  virtual void Clone(ScriptComponent *targetComponent) = 0;
};

struct ScriptComponent : public ComponentT<ScriptComponent> {
  std::vector<Script *> scripts;

public:
  ScriptComponent(Entity entity) : ComponentT<ScriptComponent>(entity) {};
  template <class T, class... T_Args> inline T *InstantiateScript(T_Args... args) {
    T *script = new T(entity, args...);
    script->OnCreate();
    scripts.push_back(script);
    return script;
  }
  void UpdateScripts(Clock clock) {
    for (auto script : scripts) {
      if (!script->started) {
        script->OnStart();
        script->started = true;
      }
      script->OnUpdate(clock);
    }
  }
  ~ScriptComponent() {
    for (auto script : scripts) {
      script->OnDestroy();
      delete script;
    }
  }

  inline void CopyFrom(ScriptComponent const &other) override {
    for (auto script : other.scripts) {
      script->Clone(this);
    }
  }
};

} // namespace Engine::Core