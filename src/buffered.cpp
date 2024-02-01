#include "buffered.hpp"

Buffer* findClosestMemory(Module* leftModule) {
  while (true) {
    if (!leftModule) return nullptr;
    if (leftModule->model == modelMemory) {
      return dynamic_cast<BufferedModule*>(leftModule)->getBuffer();
    }
    auto m = leftModule->model;
    if ((m == modelRecall) ||
        (m == modelRemember) ||
        (m == modelDisplay)) {  // This will be a list soon...
      leftModule = leftModule->getLeftExpander().module;
    } else {
      return nullptr;
    }
  }
}
