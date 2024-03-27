#include "EntityDetails.h"

#include "Editor/Display.h"
#include "Publishable.h"
#include "imgui.h"

void Engine::Editor::EntityDetails::Draw() const {
  ImGui::Begin("Details");
  for (auto c : entity.GetComponents()) {
    if (Publishable *pub = dynamic_cast<Publishable *>(c)) {
      DrawPublishable(pub);
    }
  }
  ImGui::End();
}