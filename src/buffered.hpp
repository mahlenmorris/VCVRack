#pragma once
#include "plugin.hpp"

struct Buffer {
  // TODO: make stereo
  float* array;   // make this std::shared_ptr.
  int length = 0;
};

struct BufferedModule : Module {
  Buffer buffer;

  Buffer* getBuffer() {
    return &buffer;
  }
};

Buffer* findClosestMemory(Module* leftModule);
