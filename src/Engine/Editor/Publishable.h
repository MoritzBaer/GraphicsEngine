#pragma once

#include <vector>

#define PUBLISH_WITH_LABEL(object, label) Engine::Editor::_Publication::GetPublicationWithDeducedType(object, label)
#define PUBLISH(object) PUBLISH_WITH_LABEL(object, #object)

#define RANGE(min, max, step, publication) Engine::Editor::_Publication::AddRange(publication, min, max, step)
#define PUBLISH_RANGE(object, min, max, step) RANGE(min, max, step, PUBLISH(object))
#define SLIDER(min, max, step, publication)                                                                            \
  Engine::Editor::_Publication::SetStyle(RANGE(min, max, step, publication), Engine::Editor::Publication::Style::SLIDER)
#define PUBLISH_SLIDER(object, min, max, step) SLIDER(min, max, step, PUBLISH(object))

namespace Engine::Editor {
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
    COMPOSITE
  };

  enum class Style { DRAG, SLIDER, STEPPER, RADIO, COMBO, LIST };

  enum Flags { RANGE = 1 };

  const char *label;
  Type type;
  Style style;
  int flags;
  _Publication::Range<float> floatRange;
  _Publication::Range<int> intRange;
  void *referencedPointer;
};

class Publishable {
protected:
  Publishable(const char *label) : typeLabel(label) {}

public:
  const char *typeLabel;
  virtual std::vector<Publication> GetPublications() { return {}; }
};

void DrawPublication(Publication const &publication);
void DrawPublishable(Publishable *publishable);

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

} // namespace Engine::Editor
