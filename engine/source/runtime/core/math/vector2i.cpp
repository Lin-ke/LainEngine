#include "runtime/core/math/vector2i.h"
#include "vector2.h"

namespace lain
{
    const Vector2i Vector2i::ZERO(0, 0);
    const Vector2i Vector2i::UNIT_X(1, 0);
    const Vector2i Vector2i::UNIT_Y(0, 1);
    const Vector2i Vector2i::NEGATIVE_UNIT_X(-1, 0);
    const Vector2i Vector2i::NEGATIVE_UNIT_Y(0, -1);
    const Vector2i Vector2i::UNIT_SCALE(1, 1);
    Vector2i::operator Vector2() const {
        return Vector2(static_cast<float>(x), static_cast<float>(y));
    }


} // namespace lain
