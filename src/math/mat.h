#pragma once

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "vec.h"

namespace math
{
// DONT MESS WITH ME CLANG FORMAT!!!!!!
// clang-format off
template <typename T>
class Mat4
{
 public:
  Mat4() = default;

  Mat4(T c11, T c21, T c31, T c41,
       T c12, T c22, T c32, T c42,
       T c13, T c23, T c33, T c43,
       T c14, T c24, T c34, T c44)
  {
    data_[0] = c11;
    data_[1] = c21;
    data_[2] = c31;
    data_[3] = c41;
    data_[4] = c12;
    data_[5] = c22;
    data_[6] = c32;
    data_[7] = c42;
    data_[8] = c13;
    data_[9] = c23;
    data_[10] = c33;
    data_[11] = c43;
    data_[12] = c14;
    data_[13] = c24;
    data_[14] = c34;
    data_[15] = c44;
  }

  T&
  operator()(size_t m, size_t n)
  {
    assert(m * n < 16);
    return data_[n * 4 + m];
  }

  const T&
  operator()(size_t m, size_t n) const
  {
    assert(m * n < 16);
    return data_[n * 4 + m];
  }

  // Get the i'th column of the matrix as a vec4
  v4f& operator[](size_t i) {
    assert(i < 4 && i >= 0);
    return *((v4f*)(&data_[i * 4]));
  }

  bool
  operator==(const Mat4& rhs) const
  {
    return memcmp(data_, rhs.data_, 16) == 0;
  }

  bool
  operator!=(const Mat4& rhs) const
  {
    return !(data_ == rhs.data_);
  }

  Mat4
  Transpose() const
  {
    return Mat4(
      data_[0], data_[4], data_[8], data_[12],
      data_[1], data_[5], data_[9], data_[13],
      data_[2], data_[6], data_[10], data_[14],
      data_[3], data_[7], data_[11], data_[15]);
  }

  T data_[16];
};

inline void
Print4x4Matrix(const Mat4<float>& mat)
{
  printf(
      "%.3f, %.3f, %.3f, %.3f\n"
      "%.3f, %.3f, %.3f, %.3f\n"
      "%.3f, %.3f, %.3f, %.3f\n"
      "%.3f, %.3f, %.3f, %.3f\n",
      mat(0, 0), mat(0, 1), mat(0, 2), mat(0, 3), mat(1, 0), mat(1, 1),
      mat(1, 2), mat(1, 3), mat(2, 0), mat(2, 1), mat(2, 2), mat(2, 3),
      mat(3, 0), mat(3, 1), mat(3, 2), mat(3, 3));
}

// This is fun... It's impossible for a program to compile if the
// multplication is invalid.
template <typename T>
Mat4<T> operator*(const Mat4<T>& lhs, const Mat4<T>& rhs)
{
  auto& l = lhs.data_;
  auto& r = rhs.data_;
  Mat4<T> result;
  auto& d = result.data_;

  // TODO(abrunasso): Optimize. Use simd.
  d[0] = l[0] * r[0] + l[4] * r[1] + l[8] * r[2] + l[12] * r[3];
  d[1] = l[1] * r[0] + l[5] * r[1] + l[9] * r[2] + l[13] * r[3];
  d[2] = l[2] * r[0] + l[6] * r[1] + l[10] * r[2] + l[14] * r[3];
  d[3] = l[3] * r[0] + l[7] * r[1] + l[11] * r[2] + l[15] * r[3];

  d[4] = l[0] * r[4] + l[4] * r[5] + l[8] * r[6] + l[12] * r[7];
  d[5] = l[1] * r[4] + l[5] * r[5] + l[9] * r[6] + l[13] * r[7];
  d[6] = l[2] * r[4] + l[6] * r[5] + l[10] * r[6] + l[14] * r[7];
  d[7] = l[3] * r[4] + l[7] * r[5] + l[11] * r[6] + l[15] * r[7];

  d[8 ] = l[0] * r[8] + l[4] * r[9] + l[8]  * r[10] + l[12] * r[11];
  d[9 ] = l[1] * r[8] + l[5] * r[9] + l[9]  * r[10] + l[13] * r[11];
  d[10] = l[2] * r[8] + l[6] * r[9] + l[10] * r[10] + l[14] * r[11];
  d[11] = l[3] * r[8] + l[7] * r[9] + l[11] * r[10] + l[15] * r[11];

  d[12] = l[0] * r[12] + l[4] * r[13] + l[8]  * r[14] + l[12] * r[15];
  d[13] = l[1] * r[12] + l[5] * r[13] + l[9]  * r[14] + l[13] * r[15];
  d[14] = l[2] * r[12] + l[6] * r[13] + l[10] * r[14] + l[14] * r[15];
  d[15] = l[3] * r[12] + l[7] * r[13] + l[11] * r[14] + l[15] * r[15];

  return result;
}

template <typename T>
Mat4<T> operator*(const Mat4<T>& lhs, const T& rhs)
{
  Mat4<T> r;
  r.data_[0] = lhs.data_[0] * rhs;
  r.data_[1] = lhs.data_[1] * rhs;
  r.data_[2] = lhs.data_[2] * rhs;
  r.data_[3] = lhs.data_[3] * rhs;
  r.data_[4] = lhs.data_[4] * rhs;
  r.data_[5] = lhs.data_[5] * rhs;
  r.data_[6] = lhs.data_[6] * rhs;
  r.data_[7] = lhs.data_[7] * rhs;
  r.data_[8] = lhs.data_[8] * rhs;
  r.data_[9] = lhs.data_[9] * rhs;
  r.data_[10] = lhs.data_[10] * rhs;
  r.data_[11] = lhs.data_[11] * rhs;
  r.data_[12] = lhs.data_[12] * rhs;
  r.data_[13] = lhs.data_[13] * rhs;
  r.data_[14] = lhs.data_[14] * rhs;
  r.data_[15] = lhs.data_[15] * rhs;
  return r;
}

template <typename T>
Mat4<T> operator*(const T& rhs, const Mat4<T>& lhs)
{
  return lhs * rhs;
}

template <typename T>
Mat4<T>
operator/(const Mat4<T>& lhs, const T& rhs)
{
  Mat4<T> r;
  r.data_[0] = lhs.data_[0] / rhs;
  r.data_[1] = lhs.data_[1] / rhs;
  r.data_[2] = lhs.data_[2] / rhs;
  r.data_[3] = lhs.data_[3] / rhs;
  r.data_[4] = lhs.data_[4] / rhs;
  r.data_[5] = lhs.data_[5] / rhs;
  r.data_[6] = lhs.data_[6] / rhs;
  r.data_[7] = lhs.data_[7] / rhs;
  r.data_[8] = lhs.data_[8] / rhs;
  r.data_[9] = lhs.data_[9] / rhs;
  r.data_[10] = lhs.data_[10] / rhs;
  r.data_[11] = lhs.data_[11] / rhs;
  r.data_[12] = lhs.data_[12] / rhs;
  r.data_[13] = lhs.data_[13] / rhs;
  r.data_[14] = lhs.data_[14] / rhs;
  r.data_[15] = lhs.data_[15] / rhs;
  return r;
}

template <typename T>
Mat4<T>
operator+(const Mat4<T>& lhs, const T& rhs)
{
  Mat4<T> r;
  r.data_[0] = lhs.data_[0] + rhs;
  r.data_[1] = lhs.data_[1] + rhs;
  r.data_[2] = lhs.data_[2] + rhs;
  r.data_[3] = lhs.data_[3] + rhs;
  r.data_[4] = lhs.data_[4] + rhs;
  r.data_[5] = lhs.data_[5] + rhs;
  r.data_[6] = lhs.data_[6] + rhs;
  r.data_[7] = lhs.data_[7] + rhs;
  r.data_[8] = lhs.data_[8] + rhs;
  r.data_[9] = lhs.data_[9] + rhs;
  r.data_[10] = lhs.data_[10] + rhs;
  r.data_[11] = lhs.data_[11] + rhs;
  r.data_[12] = lhs.data_[12] + rhs;
  r.data_[13] = lhs.data_[13] + rhs;
  r.data_[14] = lhs.data_[14] + rhs;
  r.data_[15] = lhs.data_[15] + rhs;
  return r;
}

template <typename T>
Mat4<T>
operator+(const T& rhs, const Mat4<T>& lhs)
{
  return lhs + rhs;
}

template <typename T>
Mat4<T>
operator-(const Mat4<T>& lhs, const T& rhs)
{
  Mat4<T> r;
  r.data_[0] = lhs.data_[0] - rhs;
  r.data_[1] = lhs.data_[1] - rhs;
  r.data_[2] = lhs.data_[2] - rhs;
  r.data_[3] = lhs.data_[3] - rhs;
  r.data_[4] = lhs.data_[4] - rhs;
  r.data_[5] = lhs.data_[5] - rhs;
  r.data_[6] = lhs.data_[6] - rhs;
  r.data_[7] = lhs.data_[7] - rhs;
  r.data_[8] = lhs.data_[8] - rhs;
  r.data_[9] = lhs.data_[9] - rhs;
  r.data_[10] = lhs.data_[10] - rhs;
  r.data_[11] = lhs.data_[11] - rhs;
  r.data_[12] = lhs.data_[12] - rhs;
  r.data_[13] = lhs.data_[13] - rhs;
  r.data_[14] = lhs.data_[14] - rhs;
  r.data_[15] = lhs.data_[15] - rhs;
  return r;

}

template <typename T>
Mat4<T>
operator-(const Mat4<T>& lhs, const Mat4<T>& rhs)
{
  Mat4<T> r;
  r.data_[0] = lhs.data_[0] - rhs.data_[0];
  r.data_[1] = lhs.data_[1] - rhs.data_[1];
  r.data_[2] = lhs.data_[2] - rhs.data_[2];
  r.data_[3] = lhs.data_[3] - rhs.data_[3];
  r.data_[4] = lhs.data_[4] - rhs.data_[4];
  r.data_[5] = lhs.data_[5] - rhs.data_[5];
  r.data_[6] = lhs.data_[6] - rhs.data_[6];
  r.data_[7] = lhs.data_[7] - rhs.data_[7];
  r.data_[8] = lhs.data_[8] - rhs.data_[8];
  r.data_[9] = lhs.data_[9] - rhs.data_[9];
  r.data_[10] = lhs.data_[10] - rhs.data_[10];
  r.data_[11] = lhs.data_[11] - rhs.data_[11];
  r.data_[12] = lhs.data_[12] - rhs.data_[12];
  r.data_[13] = lhs.data_[13] - rhs.data_[13];
  r.data_[14] = lhs.data_[14] - rhs.data_[14];
  r.data_[15] = lhs.data_[15] - rhs.data_[15];
  return r;
}

using Mat4f = Mat4<float>;

// Ths matrix vector multiplication is written as if the vector was
// augmented with a 1 (x, y, z, 1). And mutiplied against a matrix
// from the right as a column vector.
//
// This is useful when multiplying transformation matrices, consider
// the following example.
//
// For translation
//
// m = [ 1 0 0 Tx
//       0 1 0 Ty
//       0 0 1 Tz
//       0 0 0 1 ]
//
// v = [ x, y, z ]
//
// m * v = [ x + Tx, y + Ty, z + Tz ]
//
// For scale
//
// m = [ Sx 0  0  0
//       0  Sy 0  0
//       0  0  Sz 0
//       0  0  0  1 ]
//
// v = [ x, y, z ]
//
// m * v = [ x * Sx, y * Sy, z * Sz ]
//
// For some rotation along x by theta
//
// m = [ 1           0           0 0
//       0  cos(theta) -sin(theta) 0
//       0  sin(theta)  cos(theta) 0
//       0           0           0 1 ]
//
// v = [ x, y, z]
//
// m * v = [ x, ycos(theta) - zsin(theta), ysin(theta) + zcos(theta) ]
template <class T>
v3f operator*(const Mat4<T>& lhs, const v3f& rhs)
{
  return v3f(
      lhs(0, 0) * rhs.x + lhs(0, 1) * rhs.y + lhs(0, 2) * rhs.z + lhs(0, 3),
      lhs(1, 0) * rhs.x + lhs(1, 1) * rhs.y + lhs(1, 2) * rhs.z + lhs(1, 3),
      lhs(2, 0) * rhs.x + lhs(2, 1) * rhs.y + lhs(2, 2) * rhs.z + lhs(2, 3));
}

// clang-format on
}  // namespace math
