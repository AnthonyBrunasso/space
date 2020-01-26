#include <cassert>

#include "mat.h"
#include "mat_ops.h"
#include "vec.h"

void
CreateIdentityMatrix()
{
  math::Mat4f identity_matrix_4 = math::CreateIdentityMatrix<float, 4>();
  math::Mat4f desired{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
  assert(identity_matrix_4 == desired);
}

void
CreateTranslationMatrix()
{
  math::Mat4f translation_matrix =
      math::CreateTranslationMatrix(v3f(1.0f, 2.0f, 3.0f));
  math::Mat4f desired{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 2.0f, 3.0f, 1.0f};
  assert(translation_matrix == desired);
}

int
main(int argc, char** argv)
{
  CreateIdentityMatrix();
  CreateTranslationMatrix();
  return 0;
}
