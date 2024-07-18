#include "EntityDetails.h"

#include "Editor/Display.h"
#include "Editor/Editor.h"
#include "Publishable.h"
#include "imgui.h"

void Engine::Editor::EntityDetails::Draw() const {
  if (ImGui::Begin("Details")) {
    for (auto c : selectedEntity.GetComponents()) {
      if (Publishable *pub = dynamic_cast<Publishable *>(c)) {
        DrawPublishable(pub);
      }
    }
    ImGui::End();
  }
}