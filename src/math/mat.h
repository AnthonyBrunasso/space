#pragma once

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "vec.h"

namespace math
{
// column major matrix
template <typename T, size_t M, size_t N>
class Mat
{
 public:
  Mat() = default;
  // This is common case for game code. A 4x4 matrix has 16 entries.
  // Each row here corresponds with a column of the matrix.
  Mat(T c11, T c21, T c31, T c41, T c12, T c22, T c32, T c42, T c13, T c23,
      T c33, T c43, T c14, T c24, T c34, T c44)
  {
    assert(16 == M * N);  // Only allowed on 4x4 matrix.
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

  void
  Init(int n, ...)
  {
    assert(n == M * N);
    va_list vl;
    va_start(vl, n);
    for (int i = 0; i < n; i++) {
      data_[i] = va_arg(vl, T);
    }
    va_end(vl);
  }

  T&
  operator()(size_t m, size_t n)
  {
    assert(m * n < M * N);
    return data_[n * M + m];
  }

  const T&
  operator()(size_t m, size_t n) const
  {
    assert(m * n < M * N);
    return data_[n * M + m];
  }

  T& operator[](size_t i) { return data_[i]; }

  size_t
  rows() const
  {
    return M;
  }
  size_t
  cols() const
  {
    return N;
  }

  bool
  operator==(const Mat& rhs) const
  {
    if (M != rhs.rows() || N != rhs.cols()) return false;
    return memcmp(data_, rhs.data_, M * N) == 0;
  }

  bool
  operator!=(const Mat& rhs) const
  {
    return !(data_ == rhs.data_);
  }

  Mat<T, N, M>
  Transpose() const
  {
    Mat<T, N, M> t;
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        t(i, j) = (*this)(j, i);
      }
    }
    return t;
  }

  T data_[M * N];
};

inline void
Print4x4Matrix(const Mat<float, 4, 4>& mat)
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
template <typename T, size_t M, size_t N, size_t P>
Mat<T, M, P> operator*(const Mat<T, M, N>& lhs, const Mat<T, N, P>& rhs)
{
  Mat<T, M, P> r;
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < P; ++j) {
      r(i, j) = T(0);
      for (int k = 0; k < N; ++k) {
        r(i, j) += lhs(i, k) * rhs(k, j);
      }
    }
  }
  return r;
}

template <typename T, size_t M, size_t N>
Mat<T, M, N> operator*(const Mat<T, M, N>& lhs, const T& rhs)
{
  Mat<T, M, N> r;
  for (size_t i = 0; i < M * N; ++i) r.data_[i] = lhs.data_[i] * rhs;
  return r;
}

template <typename T, size_t M, size_t N>
Mat<T, M, N> operator*(const T& rhs, const Mat<T, M, N>& lhs)
{
  return lhs * rhs;
}

template <typename T, size_t M, size_t N>
Mat<T, M, N>
operator/(const Mat<T, M, N>& lhs, const T& rhs)
{
  Mat<T, M, N> r;
  for (size_t i = 0; i < M * N; ++i) r.data_[i] = lhs.data_[i] / rhs;
  return r;
}

template <typename T, size_t M, size_t N>
Mat<T, M, N>
operator+(const Mat<T, M, N>& lhs, const T& rhs)
{
  Mat<T, M, N> r;
  for (size_t i = 0; i < M * N; ++i) r.data_[i] = lhs.data_[i] + rhs;
  return r;
}

template <typename T, size_t M, size_t N>
Mat<T, M, N>
operator+(const T& rhs, const Mat<T, M, N>& lhs)
{
  return lhs + rhs;
}

template <typename T, size_t M, size_t N>
Mat<T, M, N>
operator-(const Mat<T, M, N>& lhs, const T& rhs)
{
  Mat<T, M, N> r;
  for (size_t i = 0; i < M * N; ++i) r.data_[i] = lhs.data_[i] - rhs;
  return r;
}

template <typename T, size_t M, size_t N>
Mat<T, M, N>
operator-(const Mat<T, M, N>& lhs, const Mat<T, M, N>& rhs)
{
  Mat<T, M, N> r;
  for (size_t i = 0; i < M * N; ++i) r.data_[i] = lhs.data_[i] - rhs.data_[i];
  return r;
}

using Mat4i = Mat<int, 4, 4>;
using Mat4u = Mat<uint32_t, 4, 4>;
using Mat4f = Mat<float, 4, 4>;
using Mat4d = Mat<double, 4, 4>;

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
Vec3f operator*(const Mat<T, 4, 4>& lhs, const Vec3f& rhs)
{
  return Vec3f(
      lhs(0, 0) * rhs.x + lhs(0, 1) * rhs.y + lhs(0, 2) * rhs.z + lhs(0, 3),
      lhs(1, 0) * rhs.x + lhs(1, 1) * rhs.y + lhs(1, 2) * rhs.z + lhs(1, 3),
      lhs(2, 0) * rhs.x + lhs(2, 1) * rhs.y + lhs(2, 2) * rhs.z + lhs(2, 3));
}

}  // namespace math
