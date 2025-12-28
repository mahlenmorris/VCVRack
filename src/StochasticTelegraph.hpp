#pragma once

#include "plugin.hpp"

namespace StochasticTelegraph {

// Shared utility structures and classes used by multiple modules in this plugin

/////////////////////
// A shared set of configuration available to multiple modules.

enum MRUType {
  MEMORY_DIRECTORY,
  MEMORY_FILE,
  MEMORYCV_DIRECTORY,
  MEMORYCV_FILE,
};

struct MRUList {
  std::vector<std::string> entries;
  MRUType type;

  MRUList() :type{MRUType::MEMORY_DIRECTORY} {}
  explicit MRUList(MRUType mru_type) : type{mru_type} {}

  void clear() {
    entries.clear();
  }
};

// True if was able to load the list.
bool loadMRUList(MRUList* list);
void saveMRUList(const MRUList list);



} // namespace StochasticTelegraph