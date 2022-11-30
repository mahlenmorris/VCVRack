#include "../src/parser/driver.hh"
#include <chrono>
#include <stdexcept>

using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

// Returns the number of milliseconds it takes to Compute() an expression
// NNNNN times.
void testExpression(const std::string &assignment) {
  Driver drv;
  // Some default variable values to use.
  *(drv.GetVarFromName("in1")) = 2.3;
  *(drv.GetVarFromName("in2")) = -2.3;
  *(drv.GetVarFromName("in3")) = 1.234;
  *(drv.GetVarFromName("in4")) = 0.0;

  drv.parse(assignment);
  auto expr = drv.lines[0].expr1;
  time_point<Clock> start = Clock::now();
  for (int i = 0; i < 10000000; i++) {
    float foo = expr.Compute();
  }
  time_point<Clock> end = Clock::now();
  milliseconds diff = duration_cast<milliseconds>(end - start);
  std::cout << diff.count() << " ms -- " << assignment << std::endl;
}

int main(int argc, char *argv[])
{
  testExpression("f = 3 + 4 * 6");
  testExpression("f = 3 + in2 * 6");
  testExpression("f = in1 + in2 * 6");
  testExpression("f = in1 + in2 * in3");
}
