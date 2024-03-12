#include <algorithm>

int ValidPosition(int length, int position) {
  while (position < 0) {
    position += length;
  }
  while (position >= length) {
    position -= length;
  }
  return position;
}

// 'position' is on the right side of the discontinuity we are smoothing over.
// that is, it's the larger of the two indecies on either side of the discontinuity.
void get_smooth_span(float* one_array, int* left_pos, int* right_pos, int position, int allowed_width, int length) {
  // The algorithm is to start from the middle two points and look further and further away until we:
  // * Find a left-righ pair that is less than some maximimum distance apart.
  // * OR conclude that we have found the best pair available.
  // Doing it this way automatically sorts preferentially for shortest distance.

  int best_left = 0, best_right = 0;
  float best_slope = 1000.0;  // Improbably high.
  for (int distance = 0; distance < allowed_width * 2; ++distance) {
    for (int left = std::min(distance, allowed_width - 1); left >= 0; --left) {  // Walking left -> right.
      int right = distance - left;
      if (right > allowed_width - 1) {
        break;  // Can't look any further right.
      } 
      float slope = abs(one_array[ValidPosition(length, position - 1 - left)] -
                        one_array[ValidPosition(length, position + right)]) / (distance + 1);
      // std::cout << "distance = " << distance << "\n";
      // std::cout << "left = " << left << "\n";
      // std::cout << "right = " << right << "\n";
      // std::cout << "slope = " << slope << "\n";
      if (slope < best_slope) {
        best_left = left;
        best_right = right;
        best_slope = slope;
      }
      // std::cout << "best_slope = " << best_slope << "\n\n";
    }
    if (best_slope == 0.0) {  // TODO: consider making this "best_slope < [small number]"?
      break;
    }
  }
  
  *left_pos = position - 1 - best_left;  // Might be < 0;
  *right_pos = position + best_right;  // Might be >= length.
}

void smooth_region(float* one_array, int left_pos, int right_pos, int length) {
  if (right_pos - left_pos == 1) {
    return; // Nothing to do!
  }
  float start = one_array[ValidPosition(length, left_pos)];
  float slope = (start - one_array[ValidPosition(length, right_pos)]) / (right_pos - left_pos);
  for (int offset = 1; offset < right_pos - left_pos; ++offset) {
    one_array[ValidPosition(length, left_pos + offset)] = start - slope * offset;
  }
}

// Smoothes the curves in left/right_array.
// 'position' is on the right side of the discontinuity we are smoothing over.
// that is, it's the larger of the two indecies on either side of the discontinuity.
void smooth(float* left_array, float* right_array,
            int position, int allowed_width, int length) {
  int left, right;            
  get_smooth_span(left_array, &left, &right, position, allowed_width, length);
  smooth_region(left_array, left, right, length);
  get_smooth_span(right_array, &left, &right, position, allowed_width, length);
  smooth_region(right_array, left, right, length);
}
