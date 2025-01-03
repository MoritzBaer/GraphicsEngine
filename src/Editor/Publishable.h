#pragma once

#include "imgui.h"
#include <vector>

#define PUBLISH_WITH_LABEL(object, label) Editor::Publishable<decltype(value.object)>::Publish(value.object, label)
#define PUBLISH(object) PUBLISH_WITH_LABEL(object, #object)

#define RANGE(min, max, step, publication) Editor::_Publication::AddRange(publication, min, max, step)
#define PUBLISH_RANGE(object, min, max, step) RANGE(min, max, step, PUBLISH(object))
#define SLIDER(min, max, step, publication)                                                                            \
  Editor::_Publication::SetStyle(RANGE(min, max, step, publication), Editor::Publication::Style::SLIDER)
#define PUBLISH_SLIDER(object, min, max, step) SLIDER(min, max, step, PUBLISH(object))

#define COMPOSITE(...)                                                                                                 \
  Editor::Publication {                                                                                                \
    .label = label, .type = Editor::Publication::Type::COMPOSITE, .children = { __VA_ARGS__ }                          \
  }

namespace Editor {
struct Publication;
namespace _Publication {

template <typename T> struct Range {
  T min = 0;
  T max = 0;
  float step = 1;
};

template <typename T> Publication GetPublicationWithDeducedType(T &value, const char *label);

} // namespace _Publication

struct Publication {

  enum class Type {
    INTEGER1,
    INTEGER2,
    INTEGER3,
    INTEGER4,
    FLOAT1,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    TEXT,
    COLOUR_PICKER,
    TEXTURE_SELECT,
    SHADER_SELECT,
    MESH_SELECT,
    PREFAB_SELECT,
    MATERIAL_SELECT,
    ENUM,
    COMPOSITE,
    NONE
  };

  enum class Style { DRAG, SLIDER, STEPPER, RADIO, COMBO, LIST };

  enum Flags { RANGE = 1 };

  const char *label;
  Type type;
  Style style;
  int flags;
  _Publication::Range<float> floatRange;
  _Publication::Range<int> intRange;
  std::vector<Publication> children;
  void *referencedPointer;
};

template <typename T> struct Publishable {
  static constexpr char const *typeLabel = "dummy";
  inline static Publication Publish(T &object, const char *label = typeLabel) {
    return {.type = Publication::Type::NONE};
  }
};

void DrawPublication(Publication const &publication);
template <typename T> void DrawPublishable(T &publishable);

namespace _Publication {

template <typename T> inline Publication AddRange(Publication const &publication, T min, T max, float step) {
  Publication res = publication;
  auto type = publication.type;
  if (type < Publication::Type::FLOAT1) { // Type is integral
    res.intRange = {(int)min, (int)max, step};
  } else if (type < Publication::Type::TEXT) { // Type is float
    res.floatRange = {(float)min, (float)max, step};
  }
  res.flags |= Publication::Flags::RANGE;
  return res;
}

inline Publication SetStyle(Publication const &publication, Publication::Style style) {
  Publication res = publication;
  res.style = style;
  return res;
}

} // namespace _Publication

} // namespace Editor
