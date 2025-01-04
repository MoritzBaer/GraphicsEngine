#include "Script.h"

#include "Debug/Logging.h"

void Engine::Core::ScriptComponent::CopyFrom(Component const *other) {
  if (auto otherScripts = dynamic_cast<ScriptComponent const *>(other)) {
    for (auto script : otherScripts->scripts) {
      auto newScript = script->Clone();
      newScript->OnCreate();
      scripts.push_back(newScript);
    }
  } else {
    ENGINE_ERROR("Tried to copy a ScriptComponent from a non-ScriptComponent component");
  }
}