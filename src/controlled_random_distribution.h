#include <cmath>

class RandomDistribution {
public:
  enum PDFSection { BOTH, LEFT, RIGHT };

  ~RandomDistribution() {}
  // Caller is responsible for ensuring that lower_bound <= upper_bound.
  RandomDistribution(float lower_bound, float upper_bound, float distribution,
                     float bias = 0.0f, PDFSection section = BOTH) {
    this->lower_bound = lower_bound;
    this->upper_bound = upper_bound;
    this->distribution = distribution;
    this->bias_c = (bias != 0.0f) ? std::pow(4.0f, -bias) : 1.0f;
    this->pdf_section = section;
  }
  float next() const;

private:
  float lower_bound;
  float upper_bound;
  // This ranges from 0 to 4.
  // * 0 means just the mean of the lower and upper bound is chosen.
  // * 1 means a quasi-bell curve distribution is used, with most values
  //   near the mean but some values at the bounds. This is not a true bell
  //   curve, since it doesn't have infinite tails. This is my sum of 3 uniform
  //   distributions, which is quite sufficient.
  // * 2 means a uniform distribution is used, with all values between the
  //  lower and upper bound equally likely.
  // * 3 is a U-shaped distribution, with values near the bounds more likely
  //   than values near the mean.
  // * 4 means that the value is always either the lower or upper bound,
  //   with equal probability.
  // Values between these integers interpolate between the distributions
  // simply by picking a uniform random number
  // and using it to pick between the output of the two distributions.
  float distribution;

  float bias_c;

  PDFSection pdf_section;

  // Constants for the U-shaped distribution. The k parameter controls how
  // extreme the distribution is, with higher values being more extreme.
  // The divisor and exponent are derived from k.
  static constexpr double U_DIST_K = 4.0;
  static constexpr double U_DIST_DIVISOR = 16.0; // std::pow(2.0, U_DIST_K)
  static constexpr double U_DIST_EXPONENT = 0.2; // 1.0 / (U_DIST_K + 1.0)

  // TODO: consider adding a seed-based implementation of this class,
  // so that the same sequence of random numbers could be generated each time.
};