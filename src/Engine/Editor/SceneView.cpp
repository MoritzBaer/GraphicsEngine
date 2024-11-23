#include "SceneView.h"

#include "Core/SceneHierarchy.h"
#include "Editor/Display.h"
#include "Editor/Editor.h"
#include "EntityDetails.h"
#include "imgui.h"

namespace Engine::Editor {
void DrawNode(Core::SceneHierarchy::TreeNode const &node, ImGuiTreeNodeFlags flags = 0) {
  if (!node.entity.HasComponent<Display>())
    return;

  // TODO: Align text with and without arrows
  if (!node.HasChildren()) {
    flags |= ImGuiTreeNodeFlags_Leaf;
  }

  if (selectedEntity == node.entity) {
    flags |= ImGuiTreeNodeFlags_Selected;
  }

  bool opened = ImGui::TreeNodeEx(node.entity.GetComponent<Display>()->label.data(), flags);

  if (ImGui::IsItemClicked()) {
    selectedEntity = node.entity;
  }

  flags &= !ImGuiTreeNodeFlags_Selected;

  if (opened) {
    for (auto &child : node) {
      DrawNode(child, flags);
    }
    ImGui::TreePop();
  }
}

void SceneView::Draw() const {
  ImGuiTreeNodeFlags flags =
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnArrow;
  if (ImGui::Begin("Scene")) {

    for (auto const &node : sceneHierarchy) {
      DrawNode(node, flags);
    }

    ImGui::End();
  }
}

} // namespace Engine::Editor