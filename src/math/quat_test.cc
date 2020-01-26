#include <cassert>

#include "quat.h"

#define ASSERT_NEAR(a, b, delta) (a - b <= delta)

void
Initialization()
{
  math::Quat<float> q(90.0f, v3f(0.0f, 1.0f, 0.0f));
  // I guess this is a property of a versor. The sum of the components
  // squared add up to 1.
  ASSERT_NEAR(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z, 1.0f, 0.0001f);
  math::Quat<float> q2(45.0f, v3f(1.0f, 0.0f, 0.0f));
  ASSERT_NEAR(q2.w * q2.w + q2.x * q2.x + q2.y * q2.y + q2.z * q2.z, 1.0f,
              0.0001f);
  math::Quat<float> q3(20.0f, v3f(0.0f, 0.0f, 1.0f));
  ASSERT_NEAR(q3.w * q3.w + q3.x * q3.x + q3.y * q3.y + q3.z * q3.z, 1.0f,
              0.0001f);
}

void
Up()
{
  {
    math::Quat<float> up(0.f, v3f(0.f, 0.f, 1.f));
    auto u = up.Up();
    ASSERT_NEAR(u.x, 0.f, 0.00001f);
    ASSERT_NEAR(u.y, 1.f, 0.00001f);
    ASSERT_NEAR(u.z, 0.f, 0.00001f);
  }

  {
    math::Quat<float> up(180.f, v3f(0.f, 0.f, -1.f));
    auto u = up.Up();
    ASSERT_NEAR(u.x, 0.f, 0.00001f);
    ASSERT_NEAR(u.y, -1.f, 0.00001f);
    ASSERT_NEAR(u.z, 0.f, 0.00001f);
  }
}

int
main(int argc, char** argv)
{
  Initialization();
  Up();
  return 0;
}
