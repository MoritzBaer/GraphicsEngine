#include "EntityDetails.h"

#include "Display.h"
#include "Editor.h"
#include "Publishable.h"
#include "imgui.h"

void Editor::EntityDetails::Draw() const {
  if (ImGui::Begin("Details")) {
    // for (auto c : selectedEntity.GetComponents()) {
    //   if (Publishable *pub = dynamic_cast<Publishable *>(c)) {
    //     DrawPublishable(pub);
    //   }
    // }
    ImGui::End();
  }
}