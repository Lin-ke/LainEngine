#pragma once
#ifndef QUATERNION_H
#define QUATERNION_H

#include "runtime/core/math/math.h"
#include "runtime/core/meta/reflection/reflection_marcos.h"
#include <cassert>
#include "core/math/vector3.h"
#include "core/string/ustring.h"

namespace lain {
struct Matrix3x3;
struct Vector3;
class String;

REFLECTION_TYPE(Quaternion)
struct _NO_DISCARD_ Quaternion {
  REFLECTION_BODY(Quaternion);

 public:
  META(Fields)
  real_t w{1.f}, x{0.f}, y{0.f}, z{0.f};

 public:
  Quaternion() = default;
  real_t& operator[](int p_index){
    return (&w)[p_index];
  }
  const real_t& operator[](int p_index) const{
    return (&w)[p_index];
  }

  Quaternion(real_t w_, real_t x_, real_t y_, real_t z_) : w{w_}, x{x_}, y{y_}, z{z_} {}

  // q = cos (a/2) + i(x * sin(a/2)) + j(y * sin(a/2)) + k(z * sin(a/2)) 其中a表示旋转角度，(x,y,z)表示旋转轴。
  Quaternion(const Vector3& p_axis, real_t p_angle);
  Quaternion(const Quaternion& p_q) : x(p_q.x), y(p_q.y), z(p_q.z), w(p_q.w) {}
  Quaternion(const Vector3& p_v0, const Vector3& p_v1);
  /// Construct a quaternion from a rotation matrix
  explicit Quaternion(const Matrix3x3& rot) { this->fromRotationMatrix(rot); }
  /// Construct a quaternion from an angle/axis
  Quaternion(const Radian& angle, const Vector3& axis) { this->fromAngleAxis(angle, axis); }
  /// Construct a quaternion from 3 orthonormal local axes
  Quaternion(const Vector3& xaxis, const Vector3& yaxis, const Vector3& zaxis) { this->fromAxes(xaxis, yaxis, zaxis); }

  /// Pointer accessor for direct copying
  real_t* ptr() { return &w; }

  /// Pointer accessor for direct copying
  const real_t* ptr() const { return &w; }
	_FORCE_INLINE_ Vector3 xform(const Vector3 &p_v) const {
  #ifdef MATH_CHECKS
      ERR_FAIL_COND_V_MSG(!is_normalized(), p_v, "The quaternion " + operator String() + " must be normalized.");
  #endif
		Vector3 u(x, y, z);
		Vector3 uv = u.cross(p_v);
		return p_v + ((uv * w) + u.cross(uv)) * ((real_t)2);
	}

  _FORCE_INLINE_ Vector3 xform_inv(const Vector3 &p_v) const {
		return inverse().xform(p_v);
	}

  void fromRotationMatrix(const Matrix3x3& rotation);
  void toRotationMatrix(Matrix3x3& rotation) const;
  void toRotationMatrix(Matrix4x4& rotation) const;

  void fromAngleAxis(const Radian& angle, const Vector3& axis);

  static Quaternion getQuaternionFromAngleAxis(const Radian& angle, const Vector3& axis);

  void fromDirection(const Vector3& direction, const Vector3& up_direction);

  static Quaternion getQuaternionFromDirection(const Vector3& direction, const Vector3& up_direction);

  void toAngleAxis(Radian& angle, Vector3& axis) const;

  /** Constructs the quaternion using 3 axes, the axes are assumed to be orthonormal
            @See FromAxes
        */
  void fromAxes(const Vector3& x_axis, const Vector3& y_axis, const Vector3& z_axis);
  /** Gets the 3 orthonormal axes defining the quaternion. @See FromAxes */
  void toAxes(Vector3& x_axis, Vector3& y_axis, Vector3& z_axis) const;

  /** Returns the X orthonormal axis defining the quaternion. Same as doing
            xAxis = Vector3::UNIT_X * this. Also called the local X-axis
        */
  Vector3 xAxis() const;

  /** Returns the Y orthonormal axis defining the quaternion. Same as doing
            yAxis = Vector3::UNIT_Y * this. Also called the local Y-axis
        */
  Vector3 yAxis() const;

  /** Returns the Z orthonormal axis defining the quaternion. Same as doing
            zAxis = Vector3::UNIT_Z * this. Also called the local Z-axis
        */
  Vector3 zAxis() const;

  Quaternion operator+(const Quaternion& rhs) const { return Quaternion(w + rhs.w, x + rhs.x, y + rhs.y, z + rhs.z); }

  Quaternion operator-(const Quaternion& rhs) const { return Quaternion(w - rhs.w, x - rhs.x, y - rhs.y, z - rhs.z); }

  Quaternion mul(const Quaternion& rhs) const { return (*this) * rhs; }
  Quaternion operator*(const Quaternion& rhs) const;

  Quaternion operator*(real_t scalar) const { return Quaternion(w * scalar, x * scalar, y * scalar, z * scalar); }

  //// rotation of a vector by a quaternion
  Vector3 operator*(const Vector3& rhs) const;

  Quaternion operator/(real_t scalar) const {
    assert(scalar != 0.0f);
    return Quaternion(w / scalar, x / scalar, y / scalar, z / scalar);
  }

  friend Quaternion operator*(real_t scalar, const Quaternion& rhs) { return Quaternion(scalar * rhs.w, scalar * rhs.x, scalar * rhs.y, scalar * rhs.z); }

  Quaternion operator-() const { return Quaternion(-w, -x, -y, -z); }

  bool operator==(const Quaternion& rhs) const { return (rhs.x == x) && (rhs.y == y) && (rhs.z == z) && (rhs.w == w); }

  bool operator!=(const Quaternion& rhs) const { return (rhs.x != x) || (rhs.y != y) || (rhs.z != z) || (rhs.w != w); }

  /// Check whether this quaternion contains valid values
  bool isNaN() const { return Math::isNan(x) || Math::isNan(y) || Math::isNan(z) || Math::isNan(w); }

  real_t getX() const { return x; }
  real_t getY() const { return y; }
  real_t getZ() const { return z; }
  real_t getW() const { return w; }

  // functions of a quaternion
  real_t dot(const Quaternion& rkQ) const { return w * rkQ.w + x * rkQ.x + y * rkQ.y + z * rkQ.z; }

  real_t length() const { return std::sqrt(w * w + x * x + y * y + z * z); }
  real_t length_squared() const { return w * w + x * x + y * y + z * z; }

  /// Normalizes this quaternion, and returns the previous length
  void normalise(void) {
    real_t factor = 1.0f / length();
    *this = *this * factor;
  }
  L_INLINE void normalize(void) { normalise(); }
  Quaternion normalized() { return *this / length(); }

  Quaternion inverse() const  // apply to non-zero quaternion
  {
    real_t norm = w * w + x * x + y * y + z * z;
    if (norm > 0.0) {
      real_t inv_norm = 1.0f / norm;
      return Quaternion(w * inv_norm, -x * inv_norm, -y * inv_norm, -z * inv_norm);
    } else {
      // return an invalid result to flag the error
      return ZERO;
    }
  }

  /** Calculate the local roll element of this quaternion.
          @param reprojectAxis By default the method returns the 'intuitive' result
              that is, if you projected the local Y of the quaternion onto the X and
              Y axes, the angle between them is returned. If set tok_false though, the
              result is the actual yaw that will be used to implement the quaternion,
              which is the shortest possible path to get to the same orientation and
              may involve less axial rotation.
          */
  Radian getRoll(bool reproject_axis = true) const;
  /** Calculate the local pitch element of this quaternion
        @param reprojectAxis By default the method returns the 'intuitive' result
            that is, if you projected the local Z of the quaternion onto the X and
            Y axes, the angle between them is returned. If set tok_true though, the
            result is the actual yaw that will be used to implement the quaternion,
            which is the shortest possible path to get to the same orientation and
            may involve less axial rotation.
        */
  Radian getPitch(bool reproject_axis = true) const;
  /** Calculate the local yaw element of this quaternion
        @param reprojectAxis By default the method returns the 'intuitive' result
            that is, if you projected the local Z of the quaternion onto the X and
            Z axes, the angle between them is returned. If set tok_true though, the
            result is the actual yaw that will be used to implement the quaternion,
            which is the shortest possible path to get to the same orientation and
            may involve less axial rotation.
        */
  Radian getYaw(bool reproject_axis = true) const;

  /** Performs Spherical linear interpolation between two quaternions, and returns the result.
            sLerp ( 0.0f, A, B ) = A
            sLerp ( 1.0f, A, B ) = B
            @returns Interpolated quaternion
            @remarks
            sLerp has the proprieties of performing the interpolation at constant
            velocity, and being torque-minimal (unless shortestPath=false).
            However, it's NOT commutative, which means
            Slerp ( 0.75f, A, B ) != Slerp ( 0.25f, B, A );
            therefore be careful if your code relies in the order of the operands.
            This is specially important in IK animation.
        */
  static Quaternion sLerp(real_t t, const Quaternion& kp, const Quaternion& kq, bool shortest_path = false);

  /** Performs Normalised linear interpolation between two quaternions, and returns the result.
                nLerp ( 0.0f, A, B ) = A
                nLerp ( 1.0f, A, B ) = B
                @remarks
                nLerp is faster than sLerp.
                Nlerp has the proprieties of being commutative (@See Slerp;
                commutativity is desired in certain places, like IK animation), and
                being torque-minimal (unless shortestPath=false). However, it's performing
                the interpolation at non-constant velocity; sometimes this is desired,
                sometimes it is not. Having a non-constant velocity can produce a more
                natural rotation feeling without the need of tweaking the weights; however
                if your scene relies on the timing of the rotation or assumes it will point
                at a specific angle at a specific weight value, sLerp is a better choice.
            */
  static Quaternion nLerp(real_t t, const Quaternion& kp, const Quaternion& kq, bool shortest_path = false);

  Quaternion conjugate() const { return Quaternion(w, -x, -y, -z); }

  // special values
  static const Quaternion ZERO;
  static const Quaternion IDENTITY;

  static const real_t k_epsilon;

  bool is_normalized() const {
    return Math::is_equal_approx(length_squared(), 1, (real_t)UNIT_EPSILON);  //use less epsilon
  }

  Quaternion slerp(const Quaternion& p_to, real_t p_weight) const;
  operator String() const;
};
}  // namespace lain
#endif  // !QUATERNION_H
