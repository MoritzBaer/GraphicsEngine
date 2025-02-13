#include "SceneView.h"

#include "Core/SceneHierarchy.h"
#include "Editor/Display.h"
#include "EntityDetails.h"
#include "imgui.h"

namespace Editor {
void SceneView::DrawNode(Engine::Core::SceneHierarchy::TreeNode const &node, ImGuiTreeNodeFlags flags) const {
  if (!node.entity.HasComponent<Display>())
    return;

  // TODO: Align text with and without arrows
  if (!node.HasChildren()) {
    flags |= ImGuiTreeNodeFlags_Leaf;
  }

  if (*selectedEntity == node.entity) {
    flags |= ImGuiTreeNodeFlags_Selected;
  }

  bool opened = ImGui::TreeNodeEx(node.entity.GetComponent<Display>()->label.data(), flags);

  if (ImGui::IsItemClicked()) {
    *selectedEntity = node.entity;
  }

  flags &= !ImGuiTreeNodeFlags_Selected;

  if (opened) {
    for (auto &child : node) {
      DrawNode(child, flags);
    }
    ImGui::TreePop();
  }
}

void SceneView::DrawContent() {
  ImGuiTreeNodeFlags flags =
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnArrow;
  for (auto const &node : *sceneHierarchy) {
    DrawNode(node, flags);
  }
}

} // namespace Editor