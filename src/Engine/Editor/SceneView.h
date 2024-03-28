#pragma once

#include "Graphics/ImGUIManager.h"
#include <vector>

namespace Engine::Editor {
class EntityDetails;
class SceneView : public Graphics::ImGUIView {
  std::vector<EntityDetails *> entityDetailViews;

public:
  SceneView() : ImGUIView() {}
  void Draw() const;
  inline void RegisterEntityDetails(EntityDetails *entityDetails) { entityDetailViews.push_back(entityDetails);  }
};

} // namespace Engine::Editor
