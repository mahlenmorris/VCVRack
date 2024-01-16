#include "buffered.hpp"

Buffer* findClosestMemory(Module* leftModule) {
  while (true) {
    if (!leftModule) return nullptr;
    if (leftModule->model == modelMemory) {
      return dynamic_cast<BufferedModule*>(leftModule)->getBuffer();
    }
    if ((leftModule->model == modelRecall) ||
        (leftModule->model == modelRemember)) {  // This will be a list soon...
      leftModule = leftModule->getLeftExpander().module;
    } else {
      return nullptr;
    }
  }
}
