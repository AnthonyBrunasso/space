#include <cassert>

#include <cstdio>

#include "mat.h"
#include "vec.h"

#define ASSERT_EQ(a, b) (assert(a == b))

void
MatrixIndexing()
{
  math::Mat4f m1(1.0f, 0.0f, 2.0f, 0.0f,
                 0.0f, 1.0f, 0.0f, 0.0f,
                 3.0f, 0.0f, 1.0f, 0.0f,
                 0.0f, 0.0f, 0.0f, 1.0f);
  math::Mat4f m2(1.0f, 0.0f, 2.0f, 0.0f,
                 0.0f, 1.0f, 0.0f, 0.0f,
                 3.0f, 0.0f, 1.0f, 0.0f,
                 0.0f, 0.0f, 0.0f, 1.0f);
  ASSERT_EQ(m1[0], v4f(1.0f, 0.0f, 2.0f, 0.0f));
  ASSERT_EQ(m1[1], v4f(0.0f, 1.0f, 0.0f, 0.0f));
  ASSERT_EQ(m1[2], v4f(3.0f, 0.0f, 1.0f, 0.0f));
  ASSERT_EQ(m1[3], v4f(0.0f, 0.0f, 0.0f, 1.0f));
}

void
MatrixMultiplication()
{
  math::Mat4f m1(1.0f, 0.0f, 2.0f, 0.0f,
                 0.0f, 1.0f, 0.0f, 0.0f,
                 3.0f, 0.0f, 1.0f, 0.0f,
                 0.0f, 0.0f, 0.0f, 1.0f);
  math::Mat4f m2(1.0f, 0.0f, 2.0f, 0.0f,
                 0.0f, 1.0f, 0.0f, 0.0f,
                 3.0f, 0.0f, 1.0f, 0.0f,
                 0.0f, 0.0f, 0.0f, 1.0f);
  math::Mat4f r = m1 * m2;
  math::Mat4f d(
    7.0f, 0.0f, 4.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    6.0f, 0.0f, 7.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);
  ASSERT_EQ(r, d);
}

int
main(int argc, char** argv)
{
  MatrixIndexing();
  MatrixMultiplication();
  return 0;
}
