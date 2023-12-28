#include <cmath>
#include <stdexcept>
#include <iostream>

// Researching how the BASICally mod() function fails.

int main(int argc, char *argv[])
{
  float denom = 6 * 60 * 48000.0;
  float prev_answer = -1.23;
  int i = 0;
  // If we use a float as the array index, at 1.67772e+07 you cannot add 1.0 to it
  // anymore, because we've hit the accuracy limit of a float.
  // for (float numer = 0.0; numer <= denom; numer = numer + 1.0) {
  for (double numer = 0.0; numer <= denom; numer = numer + 1.0) {
    i++;
    if (i == 100000) {
      std::cout << numer << "\n";
      i = 0;
    }
    float answer = fmod(numer, denom);
    if (prev_answer >= answer) {
      std::cout << "Failed at numer = " << numer;
      return 1;
    }
  }
  std::cout << "No error seen.";

  // Eliminate unused parameter warnigns.
  (void)argc;
  (void)argv[0];
}
