#include <cassert>

#include "mat.h"

#define ASSERT_EQ(a, b) (assert(a == b))

void
MatrixInitialization()
{
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

void
MatrixDivision()
{
}

void
MatrixAddition()
{
}

void
MatrixSubtraction()
{
}

void
MatrixVectorMultiplyTranslate()
{
}

void
MatrixVectorMultiplyScale()
{
}

int
main(int argc, char** argv)
{
  //MatrixInitialization();
  MatrixMultiplication();
  //MatrixDivision();
  //MatrixAddition();
  //MatrixSubtraction();
  //MatrixVectorMultiplyTranslate();
  //MatrixVectorMultiplyScale();
  return 0;
}
