#include "Publishable.h"

#include "Maths/Dimension.h"
#include "Maths/Quaternion.h"

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

namespace Engine::Editor::_Publication {

using Maths::Dimension1;
using Maths::Dimension2;
using Maths::Dimension3;
using Maths::Dimension4;
using Maths::Quaternion;
using Maths::Vector2;
using Maths::Vector3;
using Maths::Vector4;

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

} // namespace Engine::Editor::_Publication