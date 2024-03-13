#pragma once
#ifndef __VECTOR3I_H__
#define __VECTOR3I_H__
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
        Vector3i(int32_t x_, int32_t y_, int32_t z_) : x{ x_ }, y{ y_ }, z{ z_ } {}


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

        Vector3i operator*(float scalar) const { return Vector3i(static_cast<i32>(x * scalar), static_cast<i32>(y * scalar), static_cast<i32>(z * scalar)); }

        Vector3i operator*(const Vector3i & rhs) const { return Vector3i(x * rhs.x, y * rhs.y, z * rhs.z); }
        
        Vector3i Vector3i::operator*(const i32 p_scalar) const {
            return Vector3i(x * p_scalar, y * p_scalar, z * p_scalar);
        }

        Vector3i operator/(float scalar) const
        {
            assert(scalar != 0.0);
            return Vector3i(static_cast<i32>(x / scalar), static_cast<i32>(y / scalar), static_cast<i32>(z / scalar));
        }

        Vector3i operator/(const Vector3i & rhs) const
        {
            assert((rhs.x != 0 && rhs.y != 0 && rhs.z != 0));
            return Vector3i(x / rhs.x, y / rhs.y, z / rhs.z);
        }
        Vector3i Vector3i::operator/(const int32_t p_scalar) const {
            return Vector3i(x / p_scalar, y / p_scalar, z / p_scalar);
        }

        const Vector3i& operator+() const { return *this; }

        Vector3i operator-() const { return Vector3i(-x, -y, -z); }

        // overloaded operators to help Vector3i
        // friend function to change location
        friend Vector3i operator*(i32 scalar, const Vector3i & rhs)
        {
            return Vector3i(scalar * rhs.x, scalar * rhs.y, scalar * rhs.z);
        }

        friend Vector3i operator/(i32 scalar, const Vector3i & rhs)
        {
            assert(rhs.x != 0 && rhs.y != 0 && rhs.z != 0);
            return Vector3i(scalar / rhs.x, scalar / rhs.y, scalar / rhs.z);
        }

        friend Vector3i operator+(i32 lhs, const Vector3i & rhs)
        {
            return Vector3i(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z);
        }

        friend Vector3i operator-(i32 lhs, const Vector3i & rhs)
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
            x = static_cast<i32> (x + scalar);
            y = static_cast<i32> (y + scalar);
            z = static_cast<i32> (z + scalar);
            return *this;
        }

        Vector3i& operator+=(i32 scalar)
        {
            x += scalar;
            y += scalar;
            z += scalar;
            return *this;
        }

        Vector3i& operator-=(float scalar)
        {
            x = static_cast<i32> (x - scalar);
            y = static_cast<i32> (y - scalar);
            z = static_cast<i32> (z - scalar);
            return *this;
        }



        Vector3i& operator-=(const Vector3i & rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            return *this;
        }

        Vector3i& operator-=(i32 scalar)
        {
            x -= scalar;
            y -= scalar;
            z -= scalar;
            return *this;
        }
        
        Vector3i& operator*=(float scalar)
        {
            x = static_cast<i32>(x * scalar);
            y = static_cast<i32>(y * scalar);
            z = static_cast<i32>(z * scalar);

            return *this;
        }

        Vector3i& operator*=(const Vector3i & rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            return *this;
        }

        Vector3i& operator*=(const i32 rhs)
        {
            x *= rhs;
            y *= rhs;
            z *= rhs;
            return *this;
        }

        Vector3i& operator/=(float scalar)
        {
            assert(scalar != 0.0);
            x = static_cast<i32>(x / scalar);
            y = static_cast<i32>(y / scalar);
            z = static_cast<i32>(z / scalar);
            return *this;
        }
        // int/int是整数除法
        Vector3i& operator/=(const Vector3i & rhs)
        {
            assert(rhs.x != 0 && rhs.y != 0 && rhs.z != 0);
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            return *this;
        }
        Vector3i& operator/=(int rhs)
        {
            x /= rhs;
            y /= rhs;
            z /= rhs;
            return *this;
        }

        Vector3i& Vector3i::operator%=(const i32 p_scalar) {
            x %= p_scalar;
            y %= p_scalar;
            z %= p_scalar;
            return *this;
        }

        Vector3i Vector3i::operator%(const i32 p_scalar) const {
            return Vector3i(x % p_scalar, y % p_scalar, z % p_scalar);
        }


        /** Returns the length (magnitude) of the vector.
        @warning
        This operation requires a square root and is expensive in
        terms of CPU operations. If you don't need to know the exact
        length (e.g. for just comparing lengths) use squaredLength()
        instead.
        */

        float length() const { return static_cast<float>(std::hypot(x, y, z)); }

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
        float squaredLength() const { return static_cast<float>(x * x + y * y + z * z); }

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

        i32 dot(const Vector3i & vec) const { return x * vec.x + y * vec.y + z * vec.z; }

        /** Normalizes the vector.
        @remarks
        This method normalizes the vector such that it's
        length / magnitude is 1. The result is called a unit vector.
        @note
        This function will not crash for zero-sized vectors, but there
        will be no changes made to their components.
        @returns The previous length of the vector.
        */

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

        /** Returns true if this vector is zero length. */
        bool isZeroLength(void) const
        {
            return (x * x) + (y * y) + (z * z) == 0;
        }

        bool isZero() const { return x == 0.f && y == 0.f && z == 0.f; }



        static Vector3i clamp(const Vector3i & v, const Vector3i & min, const Vector3i & max)
        {
            return Vector3i(
                Math::clamp(v.x, min.x, max.x),Math::clamp(v.y, min.y, max.y), Math::clamp(v.z, min.z, max.z));
        }

        static int32_t getMaxElement(const Vector3i & v) { Math::getMaxElement(v.x, v.y, v.z); }
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
#endif // !__VECTOR3I_H__
