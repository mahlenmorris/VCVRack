#include <chrono>
#include <stdexcept>
#include <iostream>

#include <vector>
#include <map>
#include <unordered_map>


using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

// Researching how we might do arrays.
// Compare get and set times for a vector vs an unordered_map vs an ordered_map,
// at wildly different scales.
// Print the number of milliseconds it takes to access entries across
// methods at size 'size'.
/*
35 ms -- vec of size 100
1402 ms -- ordered_map of size 100
565 ms -- unordered_map of size 100
35 ms -- vec of size 1000
1725 ms -- ordered_map of size 1000
573 ms -- unordered_map of size 1000
35 ms -- vec of size 10000
2142 ms -- ordered_map of size 10000
580 ms -- unordered_map of size 10000
36 ms -- vec of size 100000
2615 ms -- ordered_map of size 100000
602 ms -- unordered_map of size 100000
36 ms -- vec of size 1000000
3076 ms -- ordered_map of size 1000000
627 ms -- unordered_map of size 1000000

OK, ordered_map is pretty terrible. So that's out.
Now time how long it takes to add the items.
0 ms -- vector of size 100
0 ms -- ordered_map of size 100
0 ms -- unordered_map of size 100
0 ms -- vector of size 1000
0 ms -- ordered_map of size 1000
0 ms -- unordered_map of size 1000
0 ms -- vector of size 10000
4 ms -- ordered_map of size 10000
2 ms -- unordered_map of size 10000
1 ms -- vector of size 100000
50 ms -- ordered_map of size 100000
25 ms -- unordered_map of size 100000
11 ms -- vector of size 1000000
594 ms -- ordered_map of size 1000000
251 ms -- unordered_map of size 1000000

Again, ordered_map is pointlessly slower.

I'm thinking of making this a menu option?

A) use vectors, and when adding to an index that isn't a push_back away, we add
items until the index is close.
B) use unordered_map. Slower access time, yes, but no lag due to adding millions
of items to make a vector work.

Honestly, though, I think
* usage of sparse arrays at distant indecies is very low.
* You'll never be able to add thousands of items in one sample anyway.

I think unordered_map is the place to start. At least do some timing tests
there.
*/
void testExpression(int size) {
  // Let's assume that there is NOT a gesture (like DIM) for telling
  // me how big the array is.
  std::vector<float> vec;
  std::map<int, float> ord;
  std::unordered_map<int, float> unord;

  for (int i = 0; i < size; i++) {
    // I'd prefer to just add elements in any order, but vector makes
    // that non-obvious to do.
    float value = i * 0.00001;
    vec.push_back(value);
    ord[i] = value;
    unord[i] = value;
  }

  time_point<Clock> start = Clock::now();
  for (int i = 0; i < 10000000; i++) {
    float foo = vec[i % size];
  }
  time_point<Clock> end = Clock::now();
  milliseconds diff = duration_cast<milliseconds>(end - start);
  std::cout << diff.count() << " ms -- vec of size " << size << std::endl;

  start = Clock::now();
  for (int i = 0; i < 10000000; i++) {
    float foo = ord[i % size];
  }
  end = Clock::now();
  diff = duration_cast<milliseconds>(end - start);
  std::cout << diff.count() << " ms -- ordered_map of size " << size << std::endl;

  start = Clock::now();
  for (int i = 0; i < 10000000; i++) {
    float foo = unord[i % size];
  }
  end = Clock::now();
  diff = duration_cast<milliseconds>(end - start);
  std::cout << diff.count() << " ms -- unordered_map of size " << size << std::endl;
}

void testAdds(int size) {
  // Let's assume that there is NOT a gesture (like DIM) for telling
  // me how big the array is.
  std::vector<float> vec;
  std::map<int, float> ord;
  std::unordered_map<int, float> unord;

  time_point<Clock> start = Clock::now();
  for (int i = 0; i < size; i++) {
    // I'd prefer to just add elements in any order, but vector makes
    // that non-obvious to do.
    float value = i * 0.00001;
    vec.push_back(value);
  }
  time_point<Clock> end = Clock::now();
  milliseconds diff = duration_cast<milliseconds>(end - start);
  std::cout << diff.count() << " ms -- vector of size " << size << std::endl;

  start = Clock::now();
  for (int i = 0; i < size; i++) {
    // I'd prefer to just add elements in any order, but vector makes
    // that non-obvious to do.
    float value = i * 0.00001;
    ord[i] = value;
  }
  end = Clock::now();
  diff = duration_cast<milliseconds>(end - start);
  std::cout << diff.count() << " ms -- ordered_map of size " << size << std::endl;

  start = Clock::now();
  for (int i = 0; i < size; i++) {
    // I'd prefer to just add elements in any order, but vector makes
    // that non-obvious to do.
    float value = i * 0.00001;
    unord[i] = value;
  }
  end = Clock::now();
  diff = duration_cast<milliseconds>(end - start);
  std::cout << diff.count() << " ms -- unordered_map of size " << size << std::endl;
}

int main(int argc, char *argv[])
{
  testExpression(100);
  testExpression(1000);
  testExpression(10000);
  testExpression(100000);
  testExpression(1000000);

  testAdds(100);
  testAdds(1000);
  testAdds(10000);
  testAdds(100000);
  testAdds(1000000);

}
