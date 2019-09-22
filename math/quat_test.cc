#include "quat.h"

#include "gtest/gtest.h"

TEST(QuatTest, Initialization) {
  math::Quat<float> q(90.0f, 0.0f, 1.0f, 0.0f);
  // I guess this is a property of a versor. The sum of the components
  // squared add up to 1.
  ASSERT_NEAR(
      q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3],
      1.0f, 0.0001f);
  math::Quat<float> q2(45.0f, 1.0f, 0.0f, 0.0f);
  ASSERT_NEAR(
      q2[0] * q2[0] + q2[1] * q2[1] + q2[2] * q2[2] + q2[3] * q2[3],
      1.0f, 0.0001f);
  math::Quat<float> q3(20.0f, 0.0f, 0.0f, 1.0f);
  ASSERT_NEAR(
      q3[0] * q3[0] + q3[1] * q3[1] + q3[2] * q3[2] + q3[3] * q3[3],
      1.0f, 0.0001f);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
