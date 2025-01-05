#pragma once

#include "Engine/Core/ECS.h"

namespace Engine::Core {

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
  virtual void OnUpdate(float deltaTime) {}
  virtual Script *Clone() = 0;
};

struct ScriptComponent : public Component {
  std::vector<Script *> scripts;

public:
  ScriptComponent(Entity entity) : Component(entity) {};
  template <class T, class... T_Args> inline T *InstantiateScript(T_Args... args) {
    T *script = new T(entity, args...);
    script->OnCreate();
    scripts.push_back(script);
    return script;
  }
  void UpdateScripts(float deltaTime) {
    for (auto script : scripts) {
      if (!script->started) {
        script->OnStart();
        script->started = true;
      }
      script->OnUpdate(deltaTime);
    }
  }
  ~ScriptComponent() {
    for (auto script : scripts) {
      script->OnDestroy();
      delete script;
    }
  }

  void CopyFrom(Component const *other) override;
};
} // namespace Engine::Core