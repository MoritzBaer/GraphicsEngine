#include "Publishable.h"

#include "Debug/Logging.h"
#include "Editor/Macros.h"
#include "imgui.h"

namespace Editor {

#define DRAW_DRAG_INT(func, ranged)                                                                                    \
  if (ranged) {                                                                                                        \
    func(publication.label, (int *)publication.referencedPointer, publication.intRange.step, publication.intRange.min, \
         publication.intRange.max);                                                                                    \
  } else {                                                                                                             \
    func(publication.label, (int *)publication.referencedPointer, 0.1f);                                               \
  }

#define DRAW_DRAG_FLOAT(func, ranged)                                                                                  \
  if (ranged) {                                                                                                        \
    func(publication.label, (float *)publication.referencedPointer, publication.floatRange.step,                       \
         publication.floatRange.min, publication.floatRange.max);                                                      \
  } else {                                                                                                             \
    func(publication.label, (float *)publication.referencedPointer, 0.01f);                                            \
  }

#define DRAW_SLIDER_INT(func, ranged)                                                                                  \
  func(publication.label, (int *)publication.referencedPointer, publication.intRange.min, publication.intRange.max)

#define DRAW_SLIDER_FLOAT(func, ranged)                                                                                \
  if (ranged) {                                                                                                        \
    func(publication.label, (float *)publication.referencedPointer, publication.floatRange.min,                        \
         publication.floatRange.max);                                                                                  \
  } else {                                                                                                             \
    EDITOR_ERROR("Publication labelled {} was styled as slider but the RANGE flag has not been set!",                  \
                 publication.label)                                                                                    \
  }

#define SWITCH_PUBLICATION_STYLE_INT(dragFunc, sliderFunc)                                                             \
  switch (publication.style) {                                                                                         \
  case Publication::Style::DRAG:                                                                                       \
    DRAW_DRAG_INT(dragFunc, publication.flags &Publication::Flags::RANGE);                                             \
    break;                                                                                                             \
  case Publication::Style::SLIDER:                                                                                     \
    DRAW_SLIDER_INT(sliderFunc, publication.flags &Publication::Flags::RANGE);                                         \
    break;                                                                                                             \
  case Publication::Style::STEPPER:                                                                                    \
    EDITOR_WARNING("Publication labelled {} was styled as STEPPER, which has not yet been implemented!",               \
                   publication.label)                                                                                  \
    break;                                                                                                             \
  default:                                                                                                             \
    EDITOR_ERROR("Publication labelled {} has been given an inväalid style!", publication.label)                       \
    break;                                                                                                             \
  }

#define SWITCH_PUBLICATION_STYLE_FLOAT(dragFunc, sliderFunc)                                                           \
  switch (publication.style) {                                                                                         \
  case Publication::Style::DRAG:                                                                                       \
    DRAW_DRAG_FLOAT(dragFunc, publication.flags &Publication::Flags::RANGE);                                           \
    break;                                                                                                             \
  case Publication::Style::SLIDER:                                                                                     \
    DRAW_SLIDER_FLOAT(sliderFunc, publication.flags &Publication::Flags::RANGE);                                       \
    break;                                                                                                             \
  case Publication::Style::STEPPER:                                                                                    \
    EDITOR_WARNING("Publication labelled {} was styled as STEPPER, which has not yet been implemented!",               \
                   publication.label)                                                                                  \
    break;                                                                                                             \
  default:                                                                                                             \
    EDITOR_ERROR("Publication labelled {} has been given an inväalid style!", publication.label)                       \
    break;                                                                                                             \
  }

void Editor::DrawPublication(Publication const &publication) {
  switch (publication.type) {
  case Publication::Type::INTEGER1:
    SWITCH_PUBLICATION_STYLE_INT(ImGui::DragInt, ImGui::SliderInt)
    break;

  case Publication::Type::INTEGER2:
    SWITCH_PUBLICATION_STYLE_INT(ImGui::DragInt2, ImGui::SliderInt2)
    break;

  case Publication::Type::INTEGER3:
    SWITCH_PUBLICATION_STYLE_INT(ImGui::DragInt3, ImGui::SliderInt3)
    break;

  case Publication::Type::INTEGER4:
    SWITCH_PUBLICATION_STYLE_INT(ImGui::DragInt4, ImGui::SliderInt4)
    break;

  case Publication::Type::FLOAT1:
    SWITCH_PUBLICATION_STYLE_FLOAT(ImGui::DragFloat, ImGui::SliderFloat)
    break;

  case Publication::Type::FLOAT2:
    SWITCH_PUBLICATION_STYLE_FLOAT(ImGui::DragFloat2, ImGui::SliderFloat2)
    break;

  case Publication::Type::FLOAT3:
    SWITCH_PUBLICATION_STYLE_FLOAT(ImGui::DragFloat3, ImGui::SliderFloat3)
    break;

  case Publication::Type::FLOAT4:
    // FIXME: DragFloat4 seems not to be draggable
    SWITCH_PUBLICATION_STYLE_FLOAT(ImGui::DragFloat4, ImGui::SliderFloat4)
    break;

  case Publication::Type::TEXT:
    ImGui::Text(publication.label);
    break;
  case Publication::Type::COLOUR_PICKER:
    ImGui::ColorEdit4(publication.label, (float *)publication.referencedPointer, ImGuiColorEditFlags_NoInputs);
    break;
  case Publication::Type::TEXTURE_SELECT:
    EDITOR_WARNING("Publication labelled {} has Type TEXTURE_SELECT, which has not yet been implemented!",
                   publication.label)
    break;
  case Publication::Type::SHADER_SELECT:
    EDITOR_WARNING("Publication labelled {} has Type SHADER_SELECT, which has not yet been implemented!",
                   publication.label)
    break;
  case Publication::Type::MESH_SELECT:
    EDITOR_WARNING("Publication labelled {} has Type MESH_SELECT, which has not yet been implemented!",
                   publication.label)
    break;
  case Publication::Type::PREFAB_SELECT:
    EDITOR_WARNING("Publication labelled {} has Type PREFAB_SELECT, which has not yet been implemented!",
                   publication.label)
    break;
  case Publication::Type::MATERIAL_SELECT:
    EDITOR_WARNING("Publication labelled {} has Type MATERIAL_SELECT, which has not yet been implemented!",
                   publication.label)
    break;
  case Publication::Type::ENUM:
    EDITOR_WARNING("Publication labelled {} has Type ENUM, which has not yet been implemented!", publication.label)
    break;
  case Publication::Type::COMPOSITE:
    if (ImGui::TreeNodeEx(publication.label, ImGuiTreeNodeFlags_DefaultOpen)) {
      for (auto &P : publication.children) {
        DrawPublication(P);
      }
      ImGui::TreePop();
    }
    break;
  case Publication::Type::NONE:
    break;
  default:
    EDITOR_ERROR("Publication labelled {} has an invalid type!", publication.label)
    break;
  }
}

} // namespace Editor
