#include "EntityDetails.h"

#include "Editor/Publications/Components.h"
#include "imgui.h"

void Editor::EntityDetails::Draw() {
  if (selectedEntity->IsAlive()) {
    if (ImGui::Begin("Details")) {
      ImGui::Text("Active");
      ImGui::SameLine();
      ImGui::Checkbox("##active", &active);
      if (oldActive != selectedEntity->IsActive()) {
        active = selectedEntity->IsActive();
        oldActive = active;
      } else if (oldActive != active) {
        selectedEntity->SetActive(active);
      }
    }
    for (auto c : selectedEntity->GetComponents()) {
      DrawPublication(Publishable<Engine::Core::Component *>::Publish(c));
    }
    ImGui::End();
  }
}