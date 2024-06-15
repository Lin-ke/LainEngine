#include "runtime/core/math/vector2.h"
#include "core/string/ustring.h"

namespace lain
{
    const Vector2 Vector2::ZERO(0, 0);
    const Vector2 Vector2::UNIT_X(1, 0);
    const Vector2 Vector2::UNIT_Y(0, 1);
    const Vector2 Vector2::NEGATIVE_UNIT_X(-1, 0);
    const Vector2 Vector2::NEGATIVE_UNIT_Y(0, -1);
    const Vector2 Vector2::UNIT_SCALE(1, 1);
    Vector2::operator String() const {
        return "(" + String::num_real(x, false) + ", " + String::num_real(y, false) + ")";
    }
} // namespace lain
