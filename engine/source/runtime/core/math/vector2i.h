#pragma once
#ifndef __VECTOR2I_H__
#define __VECTOR2I_H__

#include <cassert>
#include <cmath>
#include "runtime/core/math/math.h"
#include "runtime/core/meta/reflection/reflection_marcos.h"

namespace lain {
struct Vector2;
REFLECTION_TYPE(Vector2i)
STRUCT(_NO_DISCARD_ Vector2i, Fields) {
  REFLECTION_BODY(Vector2i);
  enum Axis {
    AXIS_X,
    AXIS_Y,
  };

 public:
  int32_t x{0}, y{0};

 public:
  Vector2i() = default;

  Vector2i(int x_, int y_) : x(x_), y(y_) {}
  Vector2i(const Vector2& p_vec);
  explicit Vector2i(int scaler) : x(scaler), y(scaler) {}

  explicit Vector2i(const int v[2]) : x(v[0]), y(v[1]) {}

  explicit Vector2i(int* const r) : x(r[0]), y(r[1]) {}

  int* ptr() {
    return &x;
  }

  const int* ptr() const {
    return &x;
  }

  L_INLINE int& width() {
    return x;
  }
  L_INLINE int& height() {
    return y;
  }

  int operator[](size_t i) const {
    assert(i < 2);
    return (i == 0 ? x : y);
  }

  int& operator[](size_t i) {
    assert(i < 2);
    return (i == 0 ? x : y);
  }

  operator Vector2() const;
  bool operator==(const Vector2i& rhs) const {
    return (x == rhs.x && y == rhs.y);
  }

  bool operator!=(const Vector2i& rhs) const {
    return (x != rhs.x || y != rhs.y);
  }

  // arithmetic operations
  Vector2i operator+(const Vector2i& rhs) const {
    return Vector2i(x + rhs.x, y + rhs.y);
  }

  Vector2i operator-(const Vector2i& rhs) const {
    return Vector2i(x - rhs.x, y - rhs.y);
  }

  Vector2i operator*(int scalar) const {
    return Vector2i(x * scalar, y * scalar);
  }

  Vector2i operator*(const Vector2i& rhs) const {
    return Vector2i(x * rhs.x, y * rhs.y);
  }

  Vector2i operator/(int scale) const {
    assert(scale != 0.0);

    float inv = 1.0f / scale;
    return Vector2i(static_cast<int>(x * inv), static_cast<int>(y * inv));
  }

  Vector2i operator/(const Vector2i& rhs) const {
    return Vector2i(x / rhs.x, y / rhs.y);
  }

  const Vector2i& operator+() const {
    return *this;
  }

  Vector2i operator-() const {
    return Vector2i(-x, -y);
  }

  // overloaded operators to help Vector2i
  friend Vector2i operator*(int scalar, const Vector2i& rhs) {
    return Vector2i(scalar * rhs.x, scalar * rhs.y);
  }

  friend Vector2i operator/(int fScalar, const Vector2i& rhs) {
    return Vector2i(fScalar / rhs.x, fScalar / rhs.y);
  }

  friend Vector2i operator+(const Vector2i& lhs, int rhs) {
    return Vector2i(lhs.x + rhs, lhs.y + rhs);
  }

  friend Vector2i operator+(int lhs, const Vector2i& rhs) {
    return Vector2i(lhs + rhs.x, lhs + rhs.y);
  }

  friend Vector2i operator-(const Vector2i& lhs, int rhs) {
    return Vector2i(lhs.x - rhs, lhs.y - rhs);
  }

  friend Vector2i operator-(int lhs, const Vector2i& rhs) {
    return Vector2i(lhs - rhs.x, lhs - rhs.y);
  }

  // arithmetic updates
  Vector2i& operator+=(const Vector2i& rhs) {
    x += rhs.x;
    y += rhs.y;

    return *this;
  }

  Vector2i& operator+=(int scalar) {
    x += scalar;
    y += scalar;

    return *this;
  }
  L_INLINE Vector2i sign() const {
    return Vector2i((x > 0) - (x < 0), (y > 0) - (y < 0));
  }
  Vector2i& operator-=(const Vector2i& rhs) {
    x -= rhs.x;
    y -= rhs.y;

    return *this;
  }

  Vector2i& operator-=(int scalar) {
    x -= scalar;
    y -= scalar;

    return *this;
  }

  Vector2i& operator*=(int scalar) {
    x *= scalar;
    y *= scalar;

    return *this;
  }

  Vector2i& operator*=(const Vector2i& rhs) {
    x *= rhs.x;
    y *= rhs.y;

    return *this;
  }
  Vector2i operator%(const Vector2i& p_v1) const{
	return Vector2i(x % p_v1.x, y % p_v1.y);

  }
  Vector2i operator%(int32_t p_rvalue) const{
    return Vector2i(x % p_rvalue, y % p_rvalue);
  }
  bool operator>= (const Vector2i& p_v) const{
    return x >= p_v.x ? y >= p_v.y : false;
  }
    bool operator<= (const Vector2i& p_v) const{
        return x <= p_v.x ? y <= p_v.y : false;
    }
    
  Vector2i& operator/=(int scalar) {
    assert(scalar != 0);
    x /= scalar;
    y /= scalar;

    return *this;
  }

  Vector2i& operator/=(const Vector2i& rhs) {
    x /= rhs.x;
    y /= rhs.y;

    return *this;
  }

  /** Returns the length (magnitude) of the vector.
        @warning
        This operation requires a square root and is expensive in
        terms of CPU operations. If you don't need to know the exact
        length (e.g. for just comparing lengths) use squaredLength()
        instead.
        */
  float length() const {
    return static_cast<float>(std::hypot(x, y));
  }

  /** Returns the square of the length(magnitude) of the vector.
        @remarks
        This  method is for efficiency - calculating the actual
        length of a vector requires a square root, which is expensive
        in terms of the operations required. This method returns the
        square of the length of the vector, i.e. the same as the
        length but before the square root is taken. Use this if you
        want to find the longest / shortest vector without incurring
        the square root.
        */
  int squaredLength() const {
    return x * x + y * y;
  }

  /** Returns the distance to another vector.
        @warning
        This operation requires a square root and is expensive in
        terms of CPU operations. If you don't need to know the exact
        distance (e.g. for just comparing distances) use squaredDistance()
        instead.
        */
  float distance(const Vector2i& rhs) const {
    return (*this - rhs).length();
  }

  /** Returns the square of the distance to another vector.
        @remarks
        This method is for efficiency - calculating the actual
        distance to another vector requires a square root, which is
        expensive in terms of the operations required. This method
        returns the square of the distance to another vector, i.e.
        the same as the distance but before the square root is taken.
        Use this if you want to find the longest / shortest distance
        without incurring the square root.
        */
  int squaredDistance(const Vector2i& rhs) const {
    return (*this - rhs).squaredLength();
  }

  /** Calculates the dot (scalar) product of this vector with another.
        @remarks
        The dot product can be used to calculate the angle between 2
        vectors. If both are unit vectors, the dot product is the
        cosine of the angle; otherwise the dot product must be
        divided by the product of the lengths of both vectors to get
        the cosine of the angle. This result can further be used to
        calculate the distance of a point from a plane.
        @param
        vec Vector with which to calculate the dot product (together
        with this one).
        @returns
        A int representing the dot product value.
        */
  int dot(const Vector2i& vec) const {
    return x * vec.x + y * vec.y;
  }

  /** Normalizes the vector.
        @remarks
        This method normalizes the vector such that it's
        length / magnitude is 1. The result is called a unit vector.
        @note
        This function will not crash for zero-sized vectors, but there
        will be no changes made to their components.
        @returns The previous length of the vector.
        */

  int getX() const {
    return x;
  }
  int getY() const {
    return y;
  }

  Vector2i midPoint(const Vector2i& vec) const {
    return Vector2i(static_cast<int>((x + vec.x) * 0.5f), static_cast<int>((y + vec.y) * 0.5f));
  }

  /** Returns true if the vector's scalar components are all greater
        that the ones of the vector it is compared against.
        */
  bool operator<(const Vector2i& rhs) const {
    return x < rhs.x && y < rhs.y;
  }

  /** Returns true if the vector's scalar components are all smaller
        that the ones of the vector it is compared against.
        */
  bool operator>(const Vector2i& rhs) const {
    return x > rhs.x && y > rhs.y;
  }

  /** Sets this vector's components to the minimum of its own and the
        ones of the passed in vector.
        @remarks
        'Minimum' in this case means the combination of the lowest
        value of x, y and z from both vectors. Lowest is taken just
        numerically, not magnitude, so -1 < 0.
        */
  void makeFloor(const Vector2i& cmp) {
    if (cmp.x < x)
      x = cmp.x;
    if (cmp.y < y)
      y = cmp.y;
  }

  /** Sets this vector's components to the maximum of its own and the
        ones of the passed in vector.
        @remarks
        'Maximum' in this case means the combination of the highest
        value of x, y and z from both vectors. Highest is taken just
        numerically, not magnitude, so 1 > -3.
        */
  void makeCeil(const Vector2i& cmp) {
    if (cmp.x > x)
      x = cmp.x;
    if (cmp.y > y)
      y = cmp.y;
  }

  /** Generates a vector perpendicular to this vector (eg an 'up' vector).
        @remarks
        This method will return a vector which is perpendicular to this
        vector. There are an infinite number of possibilities but this
        method will guarantee to generate one of them. If you need more
        control you should use the Quaternion class.
        */
  Vector2i perpendicular(void) const {
    return Vector2i(-y, x);
  }

  /** Calculates the 2 dimensional cross-product of 2 vectors, which results
        in a single inting point value which is 2 times the area of the triangle.
        */

  int cross(const Vector2i& rhs) const {
    return x * rhs.y - y * rhs.x;
  }

  /** Returns true if this vector is zero length. */
  bool isZeroLength(void) const {
    int sqlen = (x * x) + (y * y);
    return (sqlen < (Float_EPSILON * Float_EPSILON));
  }

  /** Calculates a reflection vector to the plane with the given normal .
        @remarks NB assumes 'this' is pointing AWAY FROM the plane, invert if it is not.
        */
  Vector2i reflect(const Vector2i& normal) const {
    return Vector2i(*this - (2 * this->dot(normal) * normal));
  }

  Vector2i min(const Vector2i& p_vector) const {
    return Vector2i(MIN(p_vector.x, p_vector.y), MIN(p_vector.x, p_vector.y));
  }

  Vector2i max(const Vector2i& p_vector) const {
    return Vector2i(MAX(p_vector.x, p_vector.y), MAX(p_vector.x, p_vector.y));
  }

  Vector2i abs() const {
    return Vector2i(Math::abs(x), Math::abs(y));
  }
  static Vector2i lerp(const Vector2i& lhs, const Vector2i& rhs, int alpha) {
    return lhs + alpha * (rhs - lhs);
  }

  Vector2i clamp(const Vector2i& p_min, const Vector2i& p_max) const;
  Vector2i snapped(const Vector2i& p_step) const;
  int64_t length_squared() const;

  operator String() const;
  // special points
  static const Vector2i ZERO;
  static const Vector2i UNIT_X;
  static const Vector2i UNIT_Y;
  static const Vector2i NEGATIVE_UNIT_X;
  static const Vector2i NEGATIVE_UNIT_Y;
  static const Vector2i UNIT_SCALE;
};

typedef Vector2i Size2i;
typedef Vector2i Point2i;

}  // namespace lain
#endif  // !VECTOR2I
