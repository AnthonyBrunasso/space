#include <cassert>

#include "mat.h"
#include "mat_ops.cc"
#include "vec.h"

void
TestInverseMatrix()
{
  math::Mat4f a(1.0f, 3.0f, 4.0f, 1.0f,
                2.0f, 1.0f, 9.0f, 1.0f,
                3.0f, 2.0f, 1.1f, 9.8f,
                1.0f, 3.0f, 0.1f, 3.2f);

  math::Mat4f inv_a = math::Inverse(a);

  // Should be identity.
  math::Print4x4Matrix(a * inv_a);


  math::Mat4f b(1.0f, 2.0f, 3.0f, 4.0f,
                5.0f, 6.0f, 7.0f, 8.0f,
                9.0f, 10.0f, 11.0f, 12.0f,
                13.0f, 14.0f, 15.0f, 16.0f);
 
  math::Mat4f r1 = b * a; 
  math::Print4x4Matrix(r1);

  math::Mat4f r2 = r1 * inv_a;
  // Should be b.
  math::Print4x4Matrix(r2);
}

int
main(int argc, char** argv)
{
  TestInverseMatrix();
  return 0;
}
