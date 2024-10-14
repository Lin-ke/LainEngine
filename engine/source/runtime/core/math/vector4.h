#pragma once

#include "runtime/core/math/math.h"
#include "runtime/core/math/vector3.h"
#include "runtime/core/meta/reflection/reflection_marcos.h"
#include "core/string/ustring.h"
namespace lain {
REFLECTION_TYPE(Vector4)
STRUCT(_NO_DISCARD_ Vector4, Fields) {
  REFLECTION_BODY(Vector4);
enum Axis {
		AXIS_X,
		AXIS_Y,
		AXIS_Z,
		AXIS_W,
	};
 public:
  real_t x{0.f}, y{0.f}, z{0.f}, w{0.f};

 public:
  Vector4() = default;
  Vector4(real_t x_, real_t y_, real_t z_, real_t w_) : x{x_}, y{y_}, z{z_}, w{w_} {}
  Vector4(const Vector3& v3, real_t w_) : x{v3.x}, y{v3.y}, z{v3.z}, w{w_} {}

  const real_t& operator[](size_t i) const  // 这里不能返回一个real_t （返回值是一个临时变量）
  {
    DEV_ASSERT(i < 4);
    return *(&x + i);
  }

  real_t& operator[](size_t i) {
    DEV_ASSERT(i < 4);
    return *(&x + i);
  }

  /// Pointer accessor for direct copying
  real_t* ptr() {
    return &x;
  }
  /// Pointer accessor for direct copying
  const real_t* ptr() const {
    return &x;
  }

  Vector4& operator=(real_t scalar) {
    x = scalar;
    y = scalar;
    z = scalar;
    w = scalar;
    return *this;
  }

  bool operator==(const Vector4& rhs) const {
    return (x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w);
  }

  bool operator!=(const Vector4& rhs) const {
    return !(rhs == *this);
  }

  Vector4 operator+(const Vector4& rhs) const {
    return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
  }
  Vector4 operator-(const Vector4& rhs) const {
    return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
  }
  Vector4 operator*(real_t scalar) const {
    return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
  }
  Vector4 operator*(const Vector4& rhs) const {
    return Vector4(rhs.x * x, rhs.y * y, rhs.z * z, rhs.w * w);
  }
  Vector4 operator/(real_t scalar) const {
    assert(scalar != 0.0);
    return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
  }
  L_INLINE Vector4 round() const {
    return Vector4(Math::round(x), Math::round(y), Math::round(z), Math::round(w));
  }
  L_INLINE Vector4 abs() const {
    return Vector4(Math::abs(x), Math::abs(y), Math::abs(z), Math::abs(w));
  }
  Vector4 operator/(const Vector4& rhs) const {
    assert(rhs.x != 0 && rhs.y != 0 && rhs.z != 0 && rhs.w != 0);
    return Vector4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w);
  }
  Vector4 sign() const{
    return Vector4(SIGN(x), SIGN(y), SIGN(z), SIGN(w));
  }

  const Vector4& operator+() const {
    return *this;
  }
	Vector4 lerp(const Vector4& p_to, real_t p_weight) const{
			Vector4 res = *this;
	res.x = Math::lerp(res.x, p_to.x, p_weight);
	res.y = Math::lerp(res.y, p_to.y, p_weight);
	res.z = Math::lerp(res.z, p_to.z, p_weight);
	res.w = Math::lerp(res.w, p_to.w, p_weight);
	return res;
	}
	Vector4 floor() const{
		return Vector4(Math::floor(x), Math::floor(y), Math::floor(z), Math::floor(w));
	}
	Vector4 ceil() const{
		return Vector4(Math::ceil(x), Math::ceil(y), Math::ceil(z), Math::ceil(w));
	}
  Vector4 operator-() const {
    return Vector4(-x, -y, -z, -w);
  }

  friend Vector4 operator*(real_t scalar, const Vector4& rhs) {
    return Vector4(scalar * rhs.x, scalar * rhs.y, scalar * rhs.z, scalar * rhs.w);
  }

  friend Vector4 operator/(real_t scalar, const Vector4& rhs) {
    assert(rhs.x != 0 && rhs.y != 0 && rhs.z != 0 && rhs.w != 0);
    return Vector4(scalar / rhs.x, scalar / rhs.y, scalar / rhs.z, scalar / rhs.w);
  }

  friend Vector4 operator+(const Vector4& lhs, real_t rhs) {
    return Vector4(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs);
  }

  friend Vector4 operator+(real_t lhs, const Vector4& rhs) {
    return Vector4(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w);
  }

  friend Vector4 operator-(const Vector4& lhs, real_t rhs) {
    return Vector4(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs);
  }

  friend Vector4 operator-(real_t lhs, const Vector4& rhs) {
    return Vector4(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z, lhs - rhs.w);
  }

  // arithmetic updates
  Vector4& operator+=(const Vector4& rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
    return *this;
  }

  Vector4& operator-=(const Vector4& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
    return *this;
  }

  Vector4& operator*=(real_t scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
  }

  Vector4& operator+=(real_t scalar) {
    x += scalar;
    y += scalar;
    z += scalar;
    w += scalar;
    return *this;
  }

  Vector4& operator-=(real_t scalar) {
    x -= scalar;
    y -= scalar;
    z -= scalar;
    w -= scalar;
    return *this;
  }

  Vector4& operator*=(const Vector4& rhs) {
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    w *= rhs.w;
    return *this;
  }

  Vector4& operator/=(real_t scalar) {
    assert(scalar != 0.0);

    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
  }

  Vector4& operator/=(const Vector4& rhs) {
    assert(rhs.x != 0 && rhs.y != 0 && rhs.z != 0);
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    w /= rhs.w;
    return *this;
  }

  /** Calculates the dot (scalar) product of this vector with another.
        @param
        vec Vector with which to calculate the dot product (together
        with this one).
        @returns
        A real_t representing the dot product value.
        */
  real_t dot(const Vector4& vec) const {
    return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
  }

  /// Check whether this vector contains valid values
  bool isNaN() const {
    return Math::isNan(x) || Math::isNan(y) || Math::isNan(z) || Math::isNan(w);
  }
  Vector4 snapped(const Vector4& p_val) const {
		return Vector4(Math::snapped(x, p_val.x), Math::snapped(y, p_val.y), Math::snapped(z, p_val.z), Math::snapped(w, p_val.w));
	}
  // special
  static const Vector4 ZERO;
  static const Vector4 UNIT_SCALE;
	// @todo 这里以后用json
	operator String() const{
		return "(" + itos(x) + ", " + itos(y) + ", " + itos(z) + itos(w) + ")";
	}
};

}  // namespace lain
