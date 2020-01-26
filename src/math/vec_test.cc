#include <cassert>

#include "vec.h"

#define ASSERT_TRUE(x) assert(x)

void
VectorAddition()
{
  v2i a(3, 4);
  v2i b(9, 2);
  ASSERT_TRUE(a.x == 3);
  ASSERT_TRUE(a.y == 4);
  ASSERT_TRUE(b.x == 9);
  ASSERT_TRUE(b.y == 2);
  auto c = a + b;
  ASSERT_TRUE(c.x == 12);
  ASSERT_TRUE(c.y == 6);
  a += b;
  ASSERT_TRUE(a.x == 12);
  ASSERT_TRUE(a.y == 6);
  math::Vec3i aa(3, 4, 9);
  math::Vec3i bb(9, 2, 5);
  ASSERT_TRUE(aa.x == 3);
  ASSERT_TRUE(aa.y == 4);
  ASSERT_TRUE(aa.z == 9);
  ASSERT_TRUE(bb.x == 9);
  ASSERT_TRUE(bb.y == 2);
  ASSERT_TRUE(bb.z == 5);
  auto cc = aa + bb;
  ASSERT_TRUE(cc.x == 12);
  ASSERT_TRUE(cc.y == 6);
  ASSERT_TRUE(cc.z == 14);
  aa += bb;
  ASSERT_TRUE(aa.x == 12);
  ASSERT_TRUE(aa.y == 6);
  ASSERT_TRUE(aa.z == 14);
}

void
VectorSubtraction()
{
  v2i a(3, 4);
  v2i b(9, 2);
  auto c = a - b;
  ASSERT_TRUE(c.x == -6);
  ASSERT_TRUE(c.y == 2);
  a -= b;
  ASSERT_TRUE(a.x == -6);
  ASSERT_TRUE(a.y == 2);
}

void
VectorScalarMultiplication()
{
  v2i a(3, 4);
  int b = 3;
  auto c = a * b;
  ASSERT_TRUE(c.x == 9);
  ASSERT_TRUE(c.y == 12);
  a *= b;
  ASSERT_TRUE(a.x == 9);
  ASSERT_TRUE(a.y == 12);
}

void
VectorScalarDivision()
{
  v2i a(9, 6);
  int b = 3;
  auto c = a / b;
  ASSERT_TRUE(c.x == 3);
  ASSERT_TRUE(c.y == 2);
  a /= b;
  ASSERT_TRUE(a.x == 3);
  ASSERT_TRUE(a.y == 2);
}

void
VectorDotProduct()
{
  v2i a(3, 4);
  v2i b(9, 2);
  ASSERT_TRUE(math::Dot(a, b) == 35);
}

void
VectorNormalization()
{
  v2f a(3.0f, 4.0f);
  v2f b = math::Normalize(a);
  ASSERT_TRUE(b.x == 0.6f);
  ASSERT_TRUE(b.y == 0.8f);
  ASSERT_TRUE(math::Length(b) == 1.0f);
}

void
VectorSquaredLength()
{
  v2i a(3, 4);
  ASSERT_TRUE(math::LengthSquared(a) == 25.0f);
}

void
VectorZeroInitialization()
{
  v2i a;
  v2f b;
  math::Vec2d c;
  ASSERT_TRUE(a.x == 0);
  ASSERT_TRUE(a.y == 0);
  ASSERT_TRUE(b.x == 0.0f);
  ASSERT_TRUE(b.y == 0.0f);
  ASSERT_TRUE(c.x == 0.0);
  ASSERT_TRUE(c.y == 0.0);
}

int
main(int argc, char** argv)
{
  VectorAddition();
  VectorSubtraction();
  VectorScalarMultiplication();
  VectorScalarDivision();
  VectorDotProduct();
  VectorNormalization();
  VectorSquaredLength();
  VectorZeroInitialization();
  return 0;
}
