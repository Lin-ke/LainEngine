
#ifndef __VECTOR4I_H__
#define __VECTOR4I_H__

#include "core/error/error_macros.h"
#include "runtime/core/math/math.h"
#include "core/meta/reflection/reflection_marcos.h"
namespace lain {

class String;
struct Vector4;
REFLECTION_TYPE(Vector4i);
STRUCT(_NO_DISCARD_ Vector4i, Fields)
{

	REFLECTION_BODY(Vector4i);
	static const int AXIS_COUNT = 4;
	enum Axis {
		AXIS_X,
		AXIS_Y,
		AXIS_Z,
		AXIS_W,
	};

	int32_t x = 0;
	int32_t y = 0;
	int32_t z = 0;
	int32_t w = 0;

	_FORCE_INLINE_ const int32_t& operator[](int p_axis) const {
		DEV_ASSERT((unsigned int)p_axis < 4);
		return *(&x + p_axis);
	}

	_FORCE_INLINE_ int32_t& operator[](int p_axis) {
		DEV_ASSERT((unsigned int)p_axis < 4);
		return *(&x + p_axis);
	}

	Vector4i::Axis min_axis_index() const;
	Vector4i::Axis max_axis_index() const;

	Vector4i min(const Vector4i& p_vector4i) const {
		return Vector4i(MIN(x, p_vector4i.x), MIN(y, p_vector4i.y), MIN(z, p_vector4i.z), MIN(w, p_vector4i.w));
	}

	Vector4i max(const Vector4i& p_vector4i) const {
		return Vector4i(MAX(x, p_vector4i.x), MAX(y, p_vector4i.y), MAX(z, p_vector4i.z), MAX(w, p_vector4i.w));
	}

	_FORCE_INLINE_ int64_t length_squared() const;
	_FORCE_INLINE_ double length() const;

	_FORCE_INLINE_ void zero();

	_FORCE_INLINE_ double distance_to(const Vector4i& p_to) const;
	_FORCE_INLINE_ int64_t distance_squared_to(const Vector4i& p_to) const;

	_FORCE_INLINE_ Vector4i abs() const;
	_FORCE_INLINE_ Vector4i sign() const;
	Vector4i clamp(const Vector4i& p_min, const Vector4i& p_max) const;
	Vector4i snapped(const Vector4i& p_step) const;

	/* Operators */

	_FORCE_INLINE_ Vector4i& operator+=(const Vector4i& p_v);
	_FORCE_INLINE_ Vector4i operator+(const Vector4i& p_v) const;
	_FORCE_INLINE_ Vector4i& operator-=(const Vector4i& p_v);
	_FORCE_INLINE_ Vector4i operator-(const Vector4i& p_v) const;
	_FORCE_INLINE_ Vector4i& operator*=(const Vector4i& p_v);
	_FORCE_INLINE_ Vector4i operator*(const Vector4i& p_v) const;
	_FORCE_INLINE_ Vector4i& operator/=(const Vector4i& p_v);
	_FORCE_INLINE_ Vector4i operator/(const Vector4i& p_v) const;
	_FORCE_INLINE_ Vector4i& operator%=(const Vector4i& p_v);
	_FORCE_INLINE_ Vector4i operator%(const Vector4i& p_v) const;

	_FORCE_INLINE_ Vector4i& operator*=(int32_t p_scalar);
	_FORCE_INLINE_ Vector4i operator*(int32_t p_scalar) const;
	_FORCE_INLINE_ Vector4i& operator/=(int32_t p_scalar);
	_FORCE_INLINE_ Vector4i operator/(int32_t p_scalar) const;
	_FORCE_INLINE_ Vector4i& operator%=(int32_t p_scalar);
	_FORCE_INLINE_ Vector4i operator%(int32_t p_scalar) const;

	_FORCE_INLINE_ Vector4i& operator*=(float p_scalar); // 乘法的计算是static_cast之后再计算
	_FORCE_INLINE_ Vector4i operator*(float p_scalar) const;

	_FORCE_INLINE_ Vector4i& operator*=(double p_scalar);
	_FORCE_INLINE_ Vector4i operator*(double p_scalar) const;

	_FORCE_INLINE_ Vector4i operator-() const;

	_FORCE_INLINE_ bool operator==(const Vector4i& p_v) const;
	_FORCE_INLINE_ bool operator!=(const Vector4i& p_v) const;
	_FORCE_INLINE_ bool operator<(const Vector4i& p_v) const;
	_FORCE_INLINE_ bool operator<=(const Vector4i& p_v) const;
	_FORCE_INLINE_ bool operator>(const Vector4i& p_v) const;
	_FORCE_INLINE_ bool operator>=(const Vector4i& p_v) const;

	operator String() const;
	operator Vector4() const;

	_FORCE_INLINE_ Vector4i() {}
	Vector4i(const Vector4& p_vec4);
	_FORCE_INLINE_ Vector4i(int32_t p_x, int32_t p_y, int32_t p_z, int32_t p_w) {
		x = p_x;
		y = p_y;
		z = p_z;
		w = p_w;
	}
};

int64_t Vector4i::length_squared() const {
	return x * (int64_t)x + y * (int64_t)y + z * (int64_t)z + w * (int64_t)w;
}

double Vector4i::length() const {
	return Math::sqrt((double)length_squared());
}

double Vector4i::distance_to(const Vector4i& p_to) const {
	return (p_to - *this).length();
}

int64_t Vector4i::distance_squared_to(const Vector4i& p_to) const {
	return (p_to - *this).length_squared();
}

/* Operators */

Vector4i& Vector4i::operator+=(const Vector4i& p_v) {
	x += p_v.x;
	y += p_v.y;
	z += p_v.z;
	w += p_v.w;
	return *this;
}

Vector4i Vector4i::operator+(const Vector4i& p_v) const {
	return Vector4i(x + p_v.x, y + p_v.y, z + p_v.z, w + p_v.w);
}

Vector4i& Vector4i::operator-=(const Vector4i& p_v) {
	x -= p_v.x;
	y -= p_v.y;
	z -= p_v.z;
	w -= p_v.w;
	return *this;
}

Vector4i Vector4i::operator-(const Vector4i& p_v) const {
	return Vector4i(x - p_v.x, y - p_v.y, z - p_v.z, w - p_v.w);
}

Vector4i& Vector4i::operator*=(const Vector4i& p_v) {
	x *= p_v.x;
	y *= p_v.y;
	z *= p_v.z;
	w *= p_v.w;
	return *this;
}

Vector4i Vector4i::operator*(const Vector4i& p_v) const {
	return Vector4i(x * p_v.x, y * p_v.y, z * p_v.z, w * p_v.w);
}

Vector4i& Vector4i::operator/=(const Vector4i& p_v) {
	x /= p_v.x;
	y /= p_v.y;
	z /= p_v.z;
	w /= p_v.w;
	return *this;
}

Vector4i Vector4i::operator/(const Vector4i& p_v) const {
	return Vector4i(x / p_v.x, y / p_v.y, z / p_v.z, w / p_v.w);
}

Vector4i& Vector4i::operator%=(const Vector4i& p_v) {
	x %= p_v.x;
	y %= p_v.y;
	z %= p_v.z;
	w %= p_v.w;
	return *this;
}

Vector4i Vector4i::operator%(const Vector4i& p_v) const {
	return Vector4i(x % p_v.x, y % p_v.y, z % p_v.z, w % p_v.w);
}

Vector4i& Vector4i::operator*=(int32_t p_scalar) {
	x *= p_scalar;
	y *= p_scalar;
	z *= p_scalar;
	w *= p_scalar;
	return *this;
}



Vector4i& Vector4i::operator*=(float p_scalar) {
	x = static_cast<int32_t>(p_scalar*x);
	y = static_cast<int32_t>(p_scalar*y);
	z = static_cast<int32_t>(p_scalar * z);
	w = static_cast<int32_t>(p_scalar * w);
}
Vector4i& Vector4i::operator*=(double p_scalar) {
	x = static_cast<int32_t>(p_scalar * x);
	y = static_cast<int32_t>(p_scalar * y);
	z = static_cast<int32_t>(p_scalar * z);
	w = static_cast<int32_t>(p_scalar * w);
}
Vector4i Vector4i::operator*(int32_t p_scalar) const {
	return Vector4i(static_cast<int32_t>(x * p_scalar), static_cast<int32_t>(y * p_scalar), static_cast<int32_t>(z * p_scalar), static_cast<int32_t>(w * p_scalar));
}

Vector4i Vector4i::operator*(float p_scalar) const {
	return Vector4i(static_cast<int32_t>(x * p_scalar), static_cast<int32_t>(y * p_scalar), static_cast<int32_t>(z * p_scalar), static_cast<int32_t>(w * p_scalar));
}

Vector4i Vector4i::operator*(double p_scalar) const {
	return Vector4i(static_cast<int32_t>(x * p_scalar), static_cast<int32_t>(y * p_scalar), static_cast<int32_t>(z * p_scalar), static_cast<int32_t>(w * p_scalar));
}
// Multiplication operators required to workaround issues with LLVM using implicit conversion.

_FORCE_INLINE_ Vector4i operator*(int32_t p_scalar, const Vector4i& p_vector) {
	return p_vector * p_scalar;
}

_FORCE_INLINE_ Vector4i operator*(int64_t p_scalar, const Vector4i& p_vector) {
	return p_vector * static_cast<int32_t>(p_scalar);
}

_FORCE_INLINE_ Vector4i operator*(float p_scalar, const Vector4i& p_vector) {
	return p_vector * p_scalar;
}

_FORCE_INLINE_ Vector4i operator*(double p_scalar, const Vector4i& p_vector) {
	return p_vector * p_scalar;
}

Vector4i& Vector4i::operator/=(int32_t p_scalar) {
	x /= p_scalar;
	y /= p_scalar;
	z /= p_scalar;
	w /= p_scalar;
	return *this;
}

Vector4i Vector4i::operator/(int32_t p_scalar) const {
	return Vector4i(x / p_scalar, y / p_scalar, z / p_scalar, w / p_scalar);
}

Vector4i& Vector4i::operator%=(int32_t p_scalar) {
	x %= p_scalar;
	y %= p_scalar;
	z %= p_scalar;
	w %= p_scalar;
	return *this;
}

Vector4i Vector4i::operator%(int32_t p_scalar) const {
	return Vector4i(x % p_scalar, y % p_scalar, z % p_scalar, w % p_scalar);
}

Vector4i Vector4i::operator-() const {
	return Vector4i(-x, -y, -z, -w);
}

bool Vector4i::operator==(const Vector4i& p_v) const {
	return (x == p_v.x && y == p_v.y && z == p_v.z && w == p_v.w);
}

bool Vector4i::operator!=(const Vector4i& p_v) const {
	return (x != p_v.x || y != p_v.y || z != p_v.z || w != p_v.w);
}

bool Vector4i::operator<(const Vector4i& p_v) const {
	if (x == p_v.x) {
		if (y == p_v.y) {
			if (z == p_v.z) {
				return w < p_v.w;
			}
			else {
				return z < p_v.z;
			}
		}
		else {
			return y < p_v.y;
		}
	}
	else {
		return x < p_v.x;
	}
}

bool Vector4i::operator>(const Vector4i& p_v) const {
	if (x == p_v.x) {
		if (y == p_v.y) {
			if (z == p_v.z) {
				return w > p_v.w;
			}
			else {
				return z > p_v.z;
			}
		}
		else {
			return y > p_v.y;
		}
	}
	else {
		return x > p_v.x;
	}
}

bool Vector4i::operator<=(const Vector4i& p_v) const {
	if (x == p_v.x) {
		if (y == p_v.y) {
			if (z == p_v.z) {
				return w <= p_v.w;
			}
			else {
				return z < p_v.z;
			}
		}
		else {
			return y < p_v.y;
		}
	}
	else {
		return x < p_v.x;
	}
}

bool Vector4i::operator>=(const Vector4i& p_v) const {
	if (x == p_v.x) {
		if (y == p_v.y) {
			if (z == p_v.z) {
				return w >= p_v.w;
			}
			else {
				return z > p_v.z;
			}
		}
		else {
			return y > p_v.y;
		}
	}
	else {
		return x > p_v.x;
	}
}

void Vector4i::zero() {
	x = y = z = w = 0;
}
}

#endif // VECTOR4I_H
