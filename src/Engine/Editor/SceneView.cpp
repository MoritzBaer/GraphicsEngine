#include "SceneView.h"

#include "Core/SceneHierarchy.h"
#include "Editor/Display.h"
#include "EntityDetails.h"
#include "imgui.h"

namespace Engine::Editor {
void DrawNode(Core::SceneHierarchy::TreeNode const &node) {
  if (!node.entity.HasComponent<Display>())
    return;

  // TODO: Align text with and without arrows
  if (node.HasChildren()) {
    if (ImGui::TreeNodeEx(node.entity.GetComponent<Display>()->label)) {
      for (auto &child : node) {
        DrawNode(child);
      }
      ImGui::TreePop();
    }
  } else {
    ImGui::Text(node.entity.GetComponent<Display>()->label);
  }
}

void SceneView::Draw() const {
  int entity = 0;
  if (ImGui::Begin("Scene")) {

    for (auto const &node : Core::SceneHierarchy::RootEntities) {
      if (entity == 1) {
        for (auto ed : entityDetailViews) {
          ed->SetEntity(node.entity);
        }
      }
      entity++;
      DrawNode(node);
    }

    ImGui::End();
  }
}

} // namespace Engine::Editor