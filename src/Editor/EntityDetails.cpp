#include "EntityDetails.h"

#include "Editor/Publications/Components.h"
#include "imgui.h"

void Editor::EntityDetails::Draw() const {
  if (ImGui::Begin("Details")) {
    for (auto c : selectedEntity->GetComponents()) {
      DrawPublication(Publishable<Engine::Core::Component *>::Publish(c));
    }
    ImGui::End();
  }
}