#include <cassert>
#include <string>
#include <cmath>
#include <random>
#include "../src/tipsy/tipsy.h"

int main(int argc, char *argv[])
{
  tipsy::ProtocolDecoder decoder;
  static constexpr size_t recvBufferSize{1024 * 64};
  unsigned char recvBuffer[recvBufferSize];

  decoder.provideDataBuffer(recvBuffer, recvBufferSize);
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(5.0,2.0);

  while (true) {
    double value = distribution(generator);
    decoder.readFloat(value);
  }
}
