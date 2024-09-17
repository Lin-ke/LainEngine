#pragma once

#include "runtime/core/math/math.h"
#include "runtime/core/math/quaternion.h"
#include "runtime/core/meta/reflection/reflection_marcos.h"

#include <cassert>

namespace lain
{
    class String;
    struct Vector3i;
    struct _NO_DISCARD_ Vector3
    {
        enum Axis {
            AXIS_X,
            AXIS_Y,
            AXIS_Z,
        };
        META(Fields)
        real_t x;
        real_t y;
        real_t z;

        void Vector3::zero() {
            x = y = z = 0;
        }
        Vector3() = default;
        Vector3(real_t x_, real_t y_, real_t z_) : x {x_}, y {y_}, z {z_} {}
        Vector3(const Vector3i& p_v);
        operator String() const;

        real_t operator[](size_t i) const
        {
            assert(i < 3);
            return *(&x + i);
        }

        real_t& operator[](size_t i)
        {
            assert(i < 3);
            return *(&x + i);
        }

        real_t& coord(size_t i) {
            assert(i < 3);
            return *(&x + i);
        }
        real_t coord(size_t i) const{
            assert(i < 3);
            return *(&x + i);
        }
        /// Pointer accessor for direct copying
        real_t* ptr() { return &x; }
        /// Pointer accessor for direct copying
        const real_t* ptr() const { return &x; }

        bool operator==(const Vector3& rhs) const { return (x == rhs.x && y == rhs.y && z == rhs.z); }

        bool operator!=(const Vector3& rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; }

        // arithmetic operations
        Vector3 operator+(const Vector3& rhs) const { return Vector3(x + rhs.x, y + rhs.y, z + rhs.z); }

        Vector3 operator-(const Vector3& rhs) const { return Vector3(x - rhs.x, y - rhs.y, z - rhs.z); }

        Vector3 operator*(real_t scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }

        Vector3 operator*(const Vector3& rhs) const { return Vector3(x * rhs.x, y * rhs.y, z * rhs.z); }

        Vector3 operator/(real_t scalar) const
        {
            assert(scalar != 0.0);
            return Vector3(x / scalar, y / scalar, z / scalar);
        }

        Vector3 operator/(const Vector3& rhs) const
        {
            assert((rhs.x != 0 && rhs.y != 0 && rhs.z != 0));
            return Vector3(x / rhs.x, y / rhs.y, z / rhs.z);
        }

        const Vector3& operator+() const { return *this; }

        Vector3 operator-() const { return Vector3(-x, -y, -z); }

        // overloaded operators to help Vector3
        friend Vector3 operator*(real_t scalar, const Vector3& rhs)
        {
            return Vector3(scalar * rhs.x, scalar * rhs.y, scalar * rhs.z);
        }

        friend Vector3 operator/(real_t scalar, const Vector3& rhs)
        {
            assert(rhs.x != 0 && rhs.y != 0 && rhs.z != 0);
            return Vector3(scalar / rhs.x, scalar / rhs.y, scalar / rhs.z);
        }

        friend Vector3 operator+(const Vector3& lhs, real_t rhs)
        {
            return Vector3(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs);
        }

        friend Vector3 operator+(real_t lhs, const Vector3& rhs)
        {
            return Vector3(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z);
        }

        friend Vector3 operator-(const Vector3& lhs, real_t rhs)
        {
            return Vector3(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs);
        }

        friend Vector3 operator-(real_t lhs, const Vector3& rhs)
        {
            return Vector3(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z);
        }

        // arithmetic updates
        Vector3& operator+=(const Vector3& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            return *this;
        }

        Vector3& operator+=(real_t scalar)
        {
            x += scalar;
            y += scalar;
            z += scalar;
            return *this;
        }

        Vector3& operator-=(const Vector3& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            return *this;
        }

        Vector3& operator-=(real_t scalar)
        {
            x -= scalar;
            y -= scalar;
            z -= scalar;
            return *this;
        }

        Vector3& operator*=(real_t scalar)
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }

        Vector3& operator*=(const Vector3& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            return *this;
        }

        Vector3& operator/=(real_t scalar)
        {
            assert(scalar != 0.0);
            x /= scalar;
            y /= scalar;
            z /= scalar;
            return *this;
        }

        Vector3& operator/=(const Vector3& rhs)
        {
            assert(rhs.x != 0 && rhs.y != 0 && rhs.z != 0);
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            return *this;
        }

        /** Returns the length (magnitude) of the vector.
        @warning
        This operation requires a square root and is expensive in
        terms of CPU operations. If you don't need to know the exact
        length (e.g. for just comparing lengths) use squaredLength()
        instead.
        */

        real_t length() const { return std::hypot(x, y, z); }

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
        real_t squaredLength() const { return x * x + y * y + z * z; }

        /** Returns the distance to another vector.
        @warning
        This operation requires a square root and is expensive in
        terms of CPU operations. If you don't need to know the exact
        distance (e.g. for just comparing distances) use squaredDistance()
        instead.
        */

        real_t distance(const Vector3& rhs) const { return (*this - rhs).length(); }

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

        real_t squaredDistance(const Vector3& rhs) const { return (*this - rhs).squaredLength(); }

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
        A real_t representing the dot product value.
        */


        real_t dot(const Vector3& p_with) const {
            return x * p_with.x + y * p_with.y + z * p_with.z;
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

        void normalise()
        {
            real_t length = std::hypot(x, y, z);
            if (length == 0.f)
                return;

            real_t inv_lengh = 1.0f / length;
            x *= inv_lengh;
            y *= inv_lengh;
            z *= inv_lengh;
        }
        L_INLINE void normalize() {
            normalise();
        }
        Vector3 normalized() const {
            Vector3 v = *this;
            v.normalize();
            return v;
        }


        /** Calculates the cross-product of 2 vectors, i.e. the vector that
        lies perpendicular to them both.
        @remarks
        The cross-product is normally used to calculate the normal
        vector of a plane, by calculating the cross-product of 2
        non-equivalent vectors which lie on the plane (e.g. 2 edges
        of a triangle).
        @param
        vec Vector which, together with this one, will be used to
        calculate the cross-product.
        @returns
        A vector which is the result of the cross-product. This
        vector will <b>NOT</b> be normalised, to maximize efficiency
        - call Vector3::normalise on the result if you wish this to
        be done. As for which side the resultant vector will be on, the
        returned vector will be on the side from which the arc from 'this'
        to rkVector is anticlockwise, e.g. UNIT_Y.cross(UNIT_Z)
        = UNIT_X, whilst UNIT_Z.cross(UNIT_Y) = -UNIT_X.
        This is because CHAOS uses a right-handed coordinate system.
        @par
        For a clearer explanation, look a the left and the bottom edges
        of your monitor's screen. Assume that the first vector is the
        left edge and the second vector is the bottom edge, both of
        them starting from the lower-left corner of the screen. The
        resulting vector is going to be perpendicular to both of them
        and will go <i>inside</i> the screen, towards the cathode tube
        (assuming you're using a CRT monitor, of course).
        */


        Vector3 Vector3::cross(const Vector3& p_with) const {
            Vector3 ret(
                (y * p_with.z) - (z * p_with.y),
                (z * p_with.x) - (x * p_with.z),
                (x * p_with.y) - (y * p_with.x));

            return ret;
        }
        

        /** Sets this vector's components to the minimum of its own and the
        ones of the passed in vector.
        @remarks
        'Minimum' in this case means the combination of the lowest
        value of x, y and z from both vectors. Lowest is taken just
        numerically, not magnitude, so -1 < 0.
        */
        void makeFloor(const Vector3& cmp)
        {
            if (cmp.x < x)
                x = cmp.x;
            if (cmp.y < y)
                y = cmp.y;
            if (cmp.z < z)
                z = cmp.z;
        }

        /** Sets this vector's components to the maximum of its own and the
        ones of the passed in vector.
        @remarks
        'Maximum' in this case means the combination of the highest
        value of x, y and z from both vectors. Highest is taken just
        numerically, not magnitude, so 1 > -3.
        */
        void makeCeil(const Vector3& cmp)
        {
            if (cmp.x > x)
                x = cmp.x;
            if (cmp.y > y)
                y = cmp.y;
            if (cmp.z > z)
                z = cmp.z;
        }

        /** Gets the angle between 2 vectors.
        @remarks
        Vectors do not have to be unit-length but must represent directions.
        */

        Radian angleBetween(const Vector3& dest) const
        {
            real_t len_product = length() * dest.length();

            // Divide by zero check
            if (len_product < 1e-6f)
                len_product = 1e-6f;

            real_t f = dot(dest) / len_product;

            f = Math::clamp(f, (real_t)-1.0, (real_t)1.0);
            return Radian(Math::acos(f));
        }
        /** Gets the shortest arc quaternion to rotate this vector to the destination
        vector.
        @remarks
        If you call this with a dest vector that is close to the inverse
        of this vector, we will rotate 180 degrees around the 'fallbackAxis'
        (if specified, or a generated axis if not) since in this case
        ANY axis of rotation is valid.
        */

        Quaternion getRotationTo(const Vector3& dest, const Vector3& fallback_axis = Vector3::ZERO) const
        {
            // Based on Stan Melax's article in Game Programming Gems
            Quaternion q;
            // Copy, since cannot modify local
            Vector3 v0 = *this;
            Vector3 v1 = dest;
            v0.normalise();
            v1.normalise();

            real_t d = v0.dot(v1);
            // If dot == 1, vectors are the same
            if (d >= 1.0f)
            {
                return Quaternion::IDENTITY;
            }
            if (d < (1e-6f - 1.0f))
            {
                if (fallback_axis != Vector3::ZERO)
                {
                    // rotate 180 degrees about the fall back axis
                    q.fromAngleAxis(Radian(Math_PIF), fallback_axis);
                }
                else
                {
                    // Generate an axis
                    Vector3 axis = Vector3::UNIT_X.cross(*this);
                    if (axis.isZeroLength()) // pick another if collinear
                        axis = Vector3::UNIT_Y.cross(*this);
                    axis.normalise();
                    q.fromAngleAxis(Radian(Math_PIF), axis);
                }
            }
            else
            {
                real_t s    = Math::sqrt((1 + d) * 2);
                real_t invs = 1 / s;

                Vector3 c = v0.cross(v1);

                q.x = c.x * invs;
                q.y = c.y * invs;
                q.z = c.z * invs;
                q.w = s * 0.5f;
                q.normalise();
            }
            return q;
        }

        L_INLINE Vector3 inverse() const {
            return Vector3(1.0f / x, 1.0f / y, 1.0f / z);
        }
        /** Returns true if this vector is zero length. */
        L_INLINE bool isZeroLength(void) const
        {
            real_t sqlen = (x * x) + (y * y) + (z * z);
            return (sqlen < (1e-06 * 1e-06));
        }

        L_INLINE bool isZero() const { return x == 0.f && y == 0.f && z == 0.f; }

        L_INLINE bool is_zero_approx() const {
            return Math::is_zero_approx(x) && Math::is_zero_approx(y) && Math::is_zero_approx(z);
        }

        /** As normalise, except that this vector is unaffected and the
        normalised vector is returned as a copy. */

        Vector3 normalisedCopy(void) const
        {
            Vector3 ret = *this;
            ret.normalise();
            return ret;
        }

        L_INLINE Vector3 lerp(const Vector3& rhs, real_t alpha) const { return Vector3::lerp(*this, rhs, alpha); }
        real_t Vector3::length_squared() const {
            real_t x2 = x * x;
            real_t y2 = y * y;
            real_t z2 = z * z;

            return x2 + y2 + z2;
        }

        /** Calculates a reflection vector to the plane with the given normal .
        @remarks NB assumes 'this' is pointing AWAY FROM the plane, invert if it is not.
        */
        Vector3 reflect(const Vector3& normal) const
        {
            return Vector3(*this - (2 * this->dot(normal) * normal));
        }

        /** Calculates projection to a plane with the given normal
        @param normal The normal of given plane
        */
        Vector3 project(const Vector3& normal) const { return Vector3(*this - (this->dot(normal) * normal)); }

        Vector3 absoluteCopy() const { return Vector3(fabsf(x), fabsf(y), fabsf(z)); }
        static Vector3 cross(const Vector3& lhs, const Vector3& rhs) {
            return lhs.cross(rhs);
        }


        static Vector3 lerp(const Vector3& lhs, const Vector3& rhs, real_t alpha) { return lhs + alpha * (rhs - lhs); }

        static Vector3 clamp(const Vector3& v, const Vector3& min, const Vector3& max)
        {
            return Vector3(
                Math::clamp(v.x, min.x, max.x), Math::clamp(v.y, min.y, max.y), Math::clamp(v.z, min.z, max.z));
        }

        static real_t getMaxElement(const Vector3& v) { return Math::getMaxElement(v.x, v.y, v.z); }
        bool         isNaN() const { return Math::isNan(x) || Math::isNan(y) || Math::isNan(z); }

        bool is_equal_approx(const Vector3& p_v) const {
            return Math::is_equal_approx(x, p_v.x) && Math::is_equal_approx(y, p_v.y) && Math::is_equal_approx(z, p_v.z);
        }

        bool is_finite() const {
            return Math::is_finite(x) && Math::is_finite(y) && Math::is_finite(z) && !is_nan();
        }
        bool is_nan() const {
            return Math::isNan(x) || Math::isNan(y) || Math::isNan(z);
        }

        Vector3 abs() const {
            return Vector3(Math::abs(x), Math::abs(y), Math::abs(z));
        }
        
        bool is_normalized() const {
            return Math::is_equal_approx(length_squared(), 1, (real_t)UNIT_EPSILON); // 这里容忍的精度大一点

        }

        void snap(const Vector3& p_step) {
            x = Math::snapped(x, p_step.x);
            y = Math::snapped(y, p_step.y);
            z = Math::snapped(z, p_step.z);
        }

        Vector3 snapped(const Vector3& p_step) const {
            Vector3 v = *this;
            v.snap(p_step);
            return v;
        }

        // special points
        static const Vector3 ZERO;
        static const Vector3 UNIT_X;
        static const Vector3 UNIT_Y;
        static const Vector3 UNIT_Z;
        static const Vector3 NEGATIVE_UNIT_X;
        static const Vector3 NEGATIVE_UNIT_Y;
        static const Vector3 NEGATIVE_UNIT_Z;
        static const Vector3 UNIT_SCALE;
    };
} // namespace lain
