#pragma once

#include "Core/ECS.h"
#include "Core/Time.h"

class Game;
namespace Engine::Core {

struct ScriptComponent;

class Script {
protected:
  Entity entity;
  Game *const& game;

public:
  bool started;
  Script(Entity entity, Game *const &game) : entity(entity), started(false), game(game) {}
  virtual ~Script() = default;
  virtual void Clone(ScriptComponent *targetComponent) = 0;

  virtual void OnCreate() {}
  virtual void OnStart() {}
  virtual void OnUpdate(Clock const &clock) {}
  virtual void OnDestroy() {}
};

struct ScriptComponent : public ComponentT<ScriptComponent> {
  std::vector<Script *> scripts;
  Game * game;

public:
  ScriptComponent(Entity entity) : ComponentT<ScriptComponent>(entity), scripts(), game(nullptr) {};
  
  inline bool IsInitialized() const { return game; }
  inline void SetGame(Game *game) { 
    this->game = game;
    for (auto & script : scripts) {
      script->OnCreate();
    } 
  }

  template <class T, class... T_Args> inline T *InstantiateScript(T_Args... args) {
    T *script = new T(entity, game, args...);
    if (game) { script->OnCreate(); }
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