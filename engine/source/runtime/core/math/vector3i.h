#pragma once

#include "runtime/core/math/math.h"
#include "runtime/core/math/quaternion.h"
#include "runtime/core/meta/reflection/reflection.h"

#include <cassert>

namespace lain
{
    class Vector3;
    REFLECTION_TYPE(Vector3i)
        CLASS(Vector3i, Fields)
    {
        REFLECTION_BODY(Vector3i);
        friend class Vector3;
    public:
        int32_t x{ 0 };
        int32_t y{ 0 };
        int32_t z{ 0 };

    public:
        Vector3i() = default;
        Vector3i(float x_, float y_, float z_) : x{ x_ }, y{ y_ }, z{ z_ } {}
        Vector3i(const Vector3& p_v);

        explicit Vector3i(const float coords[3]) : x{ coords[0] }, y{ coords[1] }, z{ coords[2] } {}

        int32_t operator[](size_t i) const
        {
            assert(i < 3);
            return *(&x + i);
        }

        int32_t& operator[](size_t i)
        {
            assert(i < 3);
            return *(&x + i);
        }
        /// Pointer accessor for direct copying
        int32_t* ptr() { return &x; }
        /// Pointer accessor for direct copying
        const int32_t* ptr() const { return &x; }

        bool operator==(const Vector3i & rhs) const { return (x == rhs.x && y == rhs.y && z == rhs.z); }

        bool operator!=(const Vector3i & rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; }

        // arithmetic operations
        Vector3i operator+(const Vector3i & rhs) const { return Vector3i(x + rhs.x, y + rhs.y, z + rhs.z); }

        Vector3i operator-(const Vector3i & rhs) const { return Vector3i(x - rhs.x, y - rhs.y, z - rhs.z); }

        Vector3i operator*(float scalar) const { return Vector3i(x * scalar, y * scalar, z * scalar); }

        Vector3i operator*(const Vector3i & rhs) const { return Vector3i(x * rhs.x, y * rhs.y, z * rhs.z); }

        Vector3i operator/(float scalar) const
        {
            assert(scalar != 0.0);
            return Vector3i(x / scalar, y / scalar, z / scalar);
        }

        Vector3i operator/(const Vector3i & rhs) const
        {
            assert((rhs.x != 0 && rhs.y != 0 && rhs.z != 0));
            return Vector3i(x / rhs.x, y / rhs.y, z / rhs.z);
        }

        const Vector3i& operator+() const { return *this; }

        Vector3i operator-() const { return Vector3i(-x, -y, -z); }

        // overloaded operators to help Vector3i
        friend Vector3i operator*(float scalar, const Vector3i & rhs)
        {
            return Vector3i(scalar * rhs.x, scalar * rhs.y, scalar * rhs.z);
        }

        friend Vector3i operator/(float scalar, const Vector3i & rhs)
        {
            assert(rhs.x != 0 && rhs.y != 0 && rhs.z != 0);
            return Vector3i(scalar / rhs.x, scalar / rhs.y, scalar / rhs.z);
        }

        friend Vector3i operator+(const Vector3i & lhs, float rhs)
        {
            return Vector3i(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs);
        }

        friend Vector3i operator+(float lhs, const Vector3i & rhs)
        {
            return Vector3i(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z);
        }

        friend Vector3i operator-(const Vector3i & lhs, float rhs)
        {
            return Vector3i(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs);
        }

        friend Vector3i operator-(float lhs, const Vector3i & rhs)
        {
            return Vector3i(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z);
        }

        // arithmetic updates
        Vector3i& operator+=(const Vector3i & rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            return *this;
        }

        Vector3i& operator+=(float scalar)
        {
            x += scalar;
            y += scalar;
            z += scalar;
            return *this;
        }

        Vector3i& operator-=(const Vector3i & rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            return *this;
        }

        Vector3i& operator-=(float scalar)
        {
            x -= scalar;
            y -= scalar;
            z -= scalar;
            return *this;
        }

        Vector3i& operator*=(float scalar)
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }

        Vector3i& operator*=(const Vector3i & rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            return *this;
        }

        Vector3i& operator/=(float scalar)
        {
            assert(scalar != 0.0);
            x /= scalar;
            y /= scalar;
            z /= scalar;
            return *this;
        }

        Vector3i& operator/=(const Vector3i & rhs)
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

        float length() const { return std::hypot(x, y, z); }

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
        float squaredLength() const { return x * x + y * y + z * z; }

        /** Returns the distance to another vector.
        @warning
        This operation requires a square root and is expensive in
        terms of CPU operations. If you don't need to know the exact
        distance (e.g. for just comparing distances) use squaredDistance()
        instead.
        */

        float distance(const Vector3i & rhs) const { return (*this - rhs).length(); }

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

        float squaredDistance(const Vector3i & rhs) const { return (*this - rhs).squaredLength(); }

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
        A float representing the dot product value.
        */

        float dotProduct(const Vector3i & vec) const { return x * vec.x + y * vec.y + z * vec.z; }

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
            float length = std::hypot(x, y, z);
            if (length == 0.f)
                return;

            float inv_lengh = 1.0f / length;
            x *= inv_lengh;
            y *= inv_lengh;
            z *= inv_lengh;
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
        - call Vector3i::normalise on the result if you wish this to
        be done. As for which side the resultant vector will be on, the
        returned vector will be on the side from which the arc from 'this'
        to rkVector is anticlockwise, e.g. UNIT_Y.crossProduct(UNIT_Z)
        = UNIT_X, whilst UNIT_Z.crossProduct(UNIT_Y) = -UNIT_X.
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

        Vector3i crossProduct(const Vector3i & rhs) const
        {
            return Vector3i(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
        }

        /** Sets this vector's components to the minimum of its own and the
        ones of the passed in vector.
        @remarks
        'Minimum' in this case means the combination of the lowest
        value of x, y and z from both vectors. Lowest is taken just
        numerically, not magnitude, so -1 < 0.
        */
        void makeFloor(const Vector3i & cmp)
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
        void makeCeil(const Vector3i & cmp)
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

        Radian angleBetween(const Vector3i & dest) const
        {
            float len_product = length() * dest.length();

            // Divide by zero check
            if (len_product < 1e-6f)
                len_product = 1e-6f;

            float f = dotProduct(dest) / len_product;

            f = Math::clamp(f, (float)-1.0, (float)1.0);
            return Math::acos(f);
        }
        /** Gets the shortest arc quaternion to rotate this vector to the destination
        vector.
        @remarks
        If you call this with a dest vector that is close to the inverse
        of this vector, we will rotate 180 degrees around the 'fallbackAxis'
        (if specified, or a generated axis if not) since in this case
        ANY axis of rotation is valid.
        */

        Quaternion getRotationTo(const Vector3i & dest, const Vector3i & fallback_axis = Vector3i::ZERO) const
        {
            // Based on Stan Melax's article in Game Programming Gems
            Quaternion q;
            // Copy, since cannot modify local
            Vector3i v0 = *this;
            Vector3i v1 = dest;
            v0.normalise();
            v1.normalise();

            float d = v0.dotProduct(v1);
            // If dot == 1, vectors are the same
            if (d >= 1.0f)
            {
                return Quaternion::IDENTITY;
            }
            if (d < (1e-6f - 1.0f))
            {
                if (fallback_axis != Vector3i::ZERO)
                {
                    // rotate 180 degrees about the fall back axis
                    q.fromAngleAxis(Radian(Math_PI), fallback_axis);
                }
                else
                {
                    // Generate an axis
                    Vector3i axis = Vector3i::UNIT_X.crossProduct(*this);
                    if (axis.isZeroLength()) // pick another if collinear
                        axis = Vector3i::UNIT_Y.crossProduct(*this);
                    axis.normalise();
                    q.fromAngleAxis(Radian(Math_PI), axis);
                }
            }
            else
            {
                float s = Math::sqrt((1 + d) * 2);
                float invs = 1 / s;

                Vector3i c = v0.crossProduct(v1);

                q.x = c.x * invs;
                q.y = c.y * invs;
                q.z = c.z * invs;
                q.w = s * 0.5f;
                q.normalise();
            }
            return q;
        }

        /** Returns true if this vector is zero length. */
        bool isZeroLength(void) const
        {
            float sqlen = (x * x) + (y * y) + (z * z);
            return (sqlen < (1e-06 * 1e-06));
        }

        bool isZero() const { return x == 0.f && y == 0.f && z == 0.f; }

        /** As normalise, except that this vector is unaffected and the
        normalised vector is returned as a copy. */

        Vector3i normalisedCopy(void) const
        {
            Vector3i ret = *this;
            ret.normalise();
            return ret;
        }

        /** Calculates a reflection vector to the plane with the given normal .
        @remarks NB assumes 'this' is pointing AWAY FROM the plane, invert if it is not.
        */
        Vector3i reflect(const Vector3i & normal) const
        {
            return Vector3i(*this - (2 * this->dotProduct(normal) * normal));
        }

        /** Calculates projection to a plane with the given normal
        @param normal The normal of given plane
        */
        Vector3i project(const Vector3i & normal) const { return Vector3i(*this - (this->dotProduct(normal) * normal)); }

        Vector3i absoluteCopy() const { return Vector3i(fabsf(x), fabsf(y), fabsf(z)); }

        static Vector3i lerp(const Vector3i & lhs, const Vector3i & rhs, float alpha) { return lhs + alpha * (rhs - lhs); }

        static Vector3i clamp(const Vector3i & v, const Vector3i & min, const Vector3i & max)
        {
            return Vector3i(
                Math::clamp(v.x, min.x, max.x), Math::clamp(v.y, min.y, max.y), Math::clamp(v.z, min.z, max.z));
        }

        static int32_t getMaxElement(const Vector3i & v) { return Math::getMaxElement(v.x, v.y, v.z); }
        bool         isNaN() const { return Math::isNan(x) || Math::isNan(y) || Math::isNan(z); }
        // special points
        static const Vector3i ZERO;
        static const Vector3i UNIT_X;
        static const Vector3i UNIT_Y;
        static const Vector3i UNIT_Z;
        static const Vector3i NEGATIVE_UNIT_X;
        static const Vector3i NEGATIVE_UNIT_Y;
        static const Vector3i NEGATIVE_UNIT_Z;
        static const Vector3i UNIT_SCALE;
    };
} // namespace lain
