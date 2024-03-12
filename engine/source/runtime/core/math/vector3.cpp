#include "runtime/core/math/vector3.h"
#include "runtime/core/math/vector3i.h"
namespace lain
{
    Vector3::Vector3(const Vector3i& p_v) {
        x = static_cast<float> (p_v.x);
        y = static_cast<float> (p_v.y);
        z = static_cast<float> (p_v.z);
    }
    const Vector3 Vector3::ZERO(0, 0, 0);
    const Vector3 Vector3::UNIT_X(1, 0, 0);
    const Vector3 Vector3::UNIT_Y(0, 1, 0);
    const Vector3 Vector3::UNIT_Z(0, 0, 1);
    const Vector3 Vector3::NEGATIVE_UNIT_X(-1, 0, 0);
    const Vector3 Vector3::NEGATIVE_UNIT_Y(0, -1, 0);
    const Vector3 Vector3::NEGATIVE_UNIT_Z(0, 0, -1);
    const Vector3 Vector3::UNIT_SCALE(1, 1, 1);
} // namespace lain
