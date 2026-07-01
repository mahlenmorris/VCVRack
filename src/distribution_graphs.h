#pragma once

#include <cstdint>

constexpr int NUM_DISTRIBUTION_GRAPHS = 41;
constexpr int NUM_BIAS_GRAPHS = 21;
constexpr int NUM_DISTRIBUTION_BINS = 61;

extern const uint8_t DISTRIBUTION_GRAPHS[NUM_DISTRIBUTION_GRAPHS][NUM_BIAS_GRAPHS][NUM_DISTRIBUTION_BINS];
