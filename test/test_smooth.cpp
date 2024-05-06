#include "gtest/gtest.h"

#include "../src/smoother.h"
#include <stdexcept>

TEST(SmoothTest, SmoothRepeatedValues)
{
  float mono[11] = { 4, 4, 3, 3, 2, /*pos*/ -5, -4, -3, -3, -2, -2};
  int left, right;

  get_smooth_span(mono, &left, &right, 5, 5, 11);
  ASSERT_EQ(0, left);  // 4 -> -2 over that distance is less of a slope. 
  ASSERT_EQ(9, right);
}

TEST(SmoothTest, SmoothZeroMid)
{
  float mono[11]   = { 4, 2, 0, -1, -2, /*pos*/ -5, -4, -3, 0, 2, 2.5};
  float result[11] = { 4, 2, 0, 0, 0, 0, 0, 0, 0, 2, 2.5};
  int left, right;

  get_smooth_span(mono, &left, &right, 5, 5, 11);
  ASSERT_EQ(2, left);
  ASSERT_EQ(8, right);
  smooth_region(mono, left, right, 11);
  for (int i = 0; i < 11; ++i) {
    EXPECT_FLOAT_EQ(result[i], mono[i]);
  }
}

TEST(SmoothTest, SmoothMinDist)
{
  float mono[11] =   { 4, 4, 4, 4, 4, /*pos*/ -4, -4, -4, -4, -4, -4};
  float result[11] = { 4, 3.1111112, 2.222222, 1.3333333, 0.44444442,
                       -0.44444466, -1.333333, -2.222222, -3.111111, -4, -4};
  int left, right;

  get_smooth_span(mono, &left, &right, 5, 5, 11);
  ASSERT_EQ(0, left);
  ASSERT_EQ(9, right);
  smooth_region(mono, left, right, 11);
  for (int i = 0; i < 11; ++i) {
    EXPECT_FLOAT_EQ(result[i], mono[i]);
  }
}

TEST(SmoothTest, SmoothOnEdge)
{
  float mono[11] =   { 4, 4, 4, 4, 4, -4, -4, -4, -4, -4, /* pos */-4};
  float result[11] = { 4, 4, 4, 4, 4, -4, -4, -4, -4, -4, -4};
  int left, right;

  get_smooth_span(mono, &left, &right, 10, 5, 11);
  ASSERT_EQ(9, left);
  ASSERT_EQ(10, right);
  smooth_region(mono, left, right, 11);
  for (int i = 0; i < 11; ++i) {
    EXPECT_FLOAT_EQ(result[i], mono[i]);
  }
}


