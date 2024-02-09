#pragma once

#include "Matrix.h"

namespace Engine::Maths
{
    template<uint8_t d>
    using Dimension = VectorT<d, uint32_t>;
    using Dimension1 = Dimension<1>;
    using Dimension2 = Dimension<2>;
    using Dimension3 = Dimension<3>;
    using Dimension4 = Dimension<4>;

} // namespace Engine::Maths
