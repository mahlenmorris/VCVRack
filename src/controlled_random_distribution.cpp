#include "controlled_random_distribution.h"
#include "plugin.hpp"
#include <cmath>
#include <algorithm>

float RandomDistribution::next() const {
  // Clamp distribution to the expected 0.0 to 4.0 range
  float dist_clamped = std::max(0.0f, std::min(4.0f, distribution));
  
  float result_0_to_1 = 0.5f; // Default to mean

  // Value morphing: interpolates Inverse-CDF equivalents instead of probability fractions.
  if (dist_clamped <= 1.0f) {
    float v1 = (random::uniform() + random::uniform() + random::uniform()) / 3.0f;
    result_0_to_1 = 0.5f + (v1 - 0.5f) * dist_clamped;
  } else if (dist_clamped <= 2.0f) {
    float u1 = random::uniform();
    float v1 = (u1 + random::uniform() + random::uniform()) / 3.0f;
    result_0_to_1 = v1 + (u1 - v1) * (dist_clamped - 1.0f);
  } else if (dist_clamped <= 3.0f) {
    float u1 = random::uniform();
    double temp_val = (u1 - 0.5) / U_DIST_DIVISOR;
    double sign = (temp_val > 0.0) ? 1.0 : ((temp_val < 0.0) ? -1.0 : 0.0);
    float v3 = static_cast<float>((sign * std::pow(std::abs(temp_val), U_DIST_EXPONENT)) + 0.5);
    result_0_to_1 = u1 + (v3 - u1) * (dist_clamped - 2.0f);
  } else {
    float u1 = random::uniform();
    double temp_val = (u1 - 0.5) / U_DIST_DIVISOR;
    double sign = (temp_val > 0.0) ? 1.0 : ((temp_val < 0.0) ? -1.0 : 0.0);
    float v3 = static_cast<float>((sign * std::pow(std::abs(temp_val), U_DIST_EXPONENT)) + 0.5);
    float v4 = (u1 >= 0.5f) ? 1.0f : 0.0f;
    result_0_to_1 = v3 + (v4 - v3) * (dist_clamped - 3.0f);
  }

  if (bias_c != 1.0f) {
    result_0_to_1 = result_0_to_1 / (result_0_to_1 + (1.0f - result_0_to_1) * bias_c);
  }

  // Fold the unwanted half into the desired half (Mirroring)
  if (pdf_section == LEFT) {
    if (result_0_to_1 > 0.5f) {
      result_0_to_1 = (1.0f - result_0_to_1);
    }
    result_0_to_1 *= 2.0f; // Scale back to [0, 1].
  } else if (pdf_section == RIGHT) {
    if (result_0_to_1 < 0.5f) {
      result_0_to_1 = (1.0f - result_0_to_1);
    }
    result_0_to_1 = (result_0_to_1 - 0.5f) * 2.0f; // Shift left and scale to [0, 1].
  }

  // Map the [0, 1] result to the requested bounds
  return lower_bound + result_0_to_1 * (upper_bound - lower_bound);
}