#ifndef __VEC_H__
#define __VEC_H__

#include <cmath>

using namespace std;

template <class T> class Vec3 {
  public:
    T x, y, z;

    Vec3() { x = y = z = 0; };

    Vec3(T x_, T y_, T z_)
    {
      x = x_;
      y = y_;
      z = z_;
    }

    constexpr T length()
    {
      return sqrt(x * x + y * y + z * z);
    }

    T normalize()
    {
      auto l = length();
      x /= l;
      y /= l;
      z /= l;
      return l;
    }

    constexpr Vec3<T> operator +(const Vec3<T> &v)
    {
      return Vec3<T>(x + v.x, y + v.y, z + v.z);
    }

    constexpr Vec3<T> operator -(const Vec3<T> &v)
    {
      return Vec3<T>(x - v.x, y - v.y, z - v.z);
    }

    // Scalar product
    constexpr Vec3<T> operator *(const T &v)
    {
      return Vec3<T>(x * v, y * v, z * v);
    }

    // Cross product
    constexpr Vec3<T> operator ^(const Vec3<T> &v)
    {
      return Vec3<T>(
          y * v.z - z * v.y,
          z * v.x - x * v.z,
          x * v.y - y * v.x);
    }

    // Dot product
    constexpr T operator *(const Vec3<T> &v)
    {
      return x * v.x + y * v.y + z * v.z;
    }

    constexpr Vec3<T> operator /(const T &v)
    {
      return Vec3<T>(x / v, y / v, z / v);
    }
};

template <class T> class Vec2 {
  public:
    T x, y;

    Vec2() { x = y = 0; }

    Vec2(T x_, T y_)
    {
      x = x_;
      y = y_;
    }
};

typedef Vec3<double> vec3;
typedef Vec2<double> vec2;
typedef Vec3<int> vec3i;
typedef Vec2<int> vec2i;

#endif //__VEC_H__
