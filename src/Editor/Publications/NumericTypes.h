#pragma once
#include "Editor/Publishable.h"

#include "Maths/Dimension.h"
#include "Maths/Quaternion.h"

#define DEDUCED_TYPE_IMPLEMENTATION_NUMERIC(ObjectType, PublicationType)                                               \
  template <> constexpr char const *Editor::Publishable<ObjectType>::typeLabel = #ObjectType;                          \
  template <> Editor::Publication Editor::Publishable<ObjectType>::Publish(ObjectType &value, const char *label) {     \
    return Publication{.label = label,                                                                                 \
                       .type = Publication::Type::PublicationType,                                                     \
                       .style = Publication::Style::DRAG,                                                              \
                       .flags = 0,                                                                                     \
                       .floatRange = {},                                                                               \
                       .intRange = {},                                                                                 \
                       .referencedPointer = &value};                                                                   \
  }

namespace Editor {

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
}