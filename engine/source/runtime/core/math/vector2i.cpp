#include "runtime/core/math/vector2i.h"
#include "vector2.h"
#include "core/string/ustring.h"

namespace lain
{
    const Vector2i Vector2i::ZERO(0, 0);
    const Vector2i Vector2i::UNIT_X(1, 0);
    const Vector2i Vector2i::UNIT_Y(0, 1);
    const Vector2i Vector2i::NEGATIVE_UNIT_X(-1, 0);
    const Vector2i Vector2i::NEGATIVE_UNIT_Y(0, -1);
    const Vector2i Vector2i::UNIT_SCALE(1, 1);
	Vector2i::Vector2i(const Vector2& p_vec) {
		x = static_cast<int>(p_vec.x);
		y = static_cast<int>(p_vec.y);

	}
    Vector2i::operator Vector2() const {
        return Vector2(static_cast<float>(x), static_cast<float>(y));
    }
	Vector2i Vector2i::clamp(const Vector2i& p_min, const Vector2i& p_max) const {
		return Vector2i(
			CLAMP(x, p_min.x, p_max.x),
			CLAMP(y, p_min.y, p_max.y));
	}

	Vector2i Vector2i::snapped(const Vector2i& p_step) const {
		return Vector2i(
			Math::snapped(x, p_step.x),
			Math::snapped(y, p_step.y));
	}

	int64_t Vector2i::length_squared() const {
		return x * (int64_t)x + y * (int64_t)y;
	}
	Vector2i::operator String() const {
		return "(" + itos(x) + ", " + itos(y) + ")";
	}


} // namespace lain
