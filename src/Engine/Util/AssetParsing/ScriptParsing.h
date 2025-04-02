#pragma once

#include "Core/Script.h"

namespace Engine {

struct ScriptDSO {
  virtual void Attach(Core::ScriptComponent *scriptComponent, AssetManager *assetManager) = 0;
};

template <typename T, typename... T_Args> struct ScriptT : public ScriptDSO {
  std::tuple<T_Args...> args;
  void Attach(Core::ScriptComponent *scriptComponent, AssetManager *assetManager) override {
    scriptComponent->InstantiateScript<T>(args...);
  }
};

} // namespace Engine