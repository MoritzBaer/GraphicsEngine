#include "Publishable.h"

#include "Debug/Logging.h"
#include "Editor/Macros.h"
#include "Maths/Dimension.h"
#include "Maths/Quaternion.h"
#include "imgui.h"

#define DEDUCED_TYPE_IMPLEMENTATION_NUMERIC(objectType, publicationType)                                               \
  template <> Publication GetPublicationWithDeducedType<objectType>(objectType & value, const char *label) {           \
    return Publication{.label = label,                                                                                 \
                       .type = Publication::Type::publicationType,                                                     \
                       .style = Publication::Style::DRAG,                                                              \
                       .flags = 0,                                                                                     \
                       .floatRange = {},                                                                               \
                       .intRange = {},                                                                                 \
                       .referencedPointer = &value};                                                                   \
  }

namespace Editor::_Publication {

using Engine::Maths::Dimension1;
using Engine::Maths::Dimension2;
using Engine::Maths::Dimension3;
using Engine::Maths::Dimension4;
using Engine::Maths::Quaternion;
using Engine::Maths::Vector2;
using Engine::Maths::Vector3;
using Engine::Maths::Vector4;

DEDUCED_TYPE_IMPLEMENTATION_NUMERIC(int, INTEGER1)

DEDUCED_TYPE_IMPLEMENTATION_NUMERIC(Dimension1, INTEGER1)
DEDUCED_TYPE_IMPLEMENTATION_NUMERIC(Dimension2, INTEGER2)
DEDUCED_TYPE_IMPLEMENTATION_NUMERIC(Dimension3, INTEGER3)
DEDUCED_TYPE_IMPLEMENTATION_NUMERIC(Dimension4, INTEGER4)

DEDUCED_TYPE_IMPLEMENTATION_NUMERIC(float, FLOAT1)

DEDUCED_TYPE_IMPLEMENTATION_NUMERIC(Vector2, FLOAT2)
DEDUCED_TYPE_IMPLEMENTATION_NUMERIC(Vector3, FLOAT3)
DEDUCED_TYPE_IMPLEMENTATION_NUMERIC(Vector4, FLOAT4)

DEDUCED_TYPE_IMPLEMENTATION_NUMERIC(Quaternion, FLOAT4)

DEDUCED_TYPE_IMPLEMENTATION_NUMERIC(const char *,
                                    TEXT) // HACK: Text isn't a numeric type, but there are no text-specific styles

} // namespace Editor::_Publication

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
      for (auto &P : reinterpret_cast<Publishable *>(publication.referencedPointer)->GetPublications()) {
        DrawPublication(P);
      }
      ImGui::TreePop();
    }
    break;
  default:
    break;
  }
}

void Editor::DrawPublishable(Publishable *publishable) {
  ImGui::BeginGroup();
  ImGui::Text(publishable->typeLabel);
  auto pubs = publishable->GetPublications();
  for (auto &P : publishable->GetPublications()) {
    DrawPublication(P);
  }
  ImGui::EndGroup();
}

} // namespace Editor
