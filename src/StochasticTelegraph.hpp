#pragma once

#include "plugin.hpp"

namespace StochasticTelegraph {

// Shared utility structures and classes used by multiple modules in this plugin.

/////////////////////
// A shared set of configuration available to multiple modules.

enum MRUType {
  MEMORY_DIRECTORY,
  MEMORY_FILE,
  MEMORYCV_DIRECTORY,
  MEMORYCV_FILE,
  MRU_LEN
};

class PluginConfig {
public:
  // Public static method to access the single instance
  static PluginConfig& getInstance() {
      // The static local variable is created only once, on the first call.
      // This initialization is guaranteed to be thread-safe in C++11 and later.
      static PluginConfig instance;
      return instance;
  }

  // Clear and fill "list" with the MRU entries of the given type.
  std::vector<std::string>* getMRUList(MRUType type);

  // Place the given entry at the top of the list of the given type.
  // May cause the config to be saved to disk, so DO NOT call in the process() loop.
  void putMRUEntry(const std::string& entry, MRUType type);

private:
  // Private constructor to prevent direct instantiation from outside
  PluginConfig();

  void saveConfigToDisk();

  // Stuff to make this work as a singleton.
  // Delete copy constructor and assignment operator to prevent cloning
  PluginConfig(const PluginConfig&) = delete;
  PluginConfig& operator=(const PluginConfig&) = delete;
  
  // Optional: Delete move constructor and move assignment operator
  PluginConfig(PluginConfig&&) = delete;
  PluginConfig& operator=(PluginConfig&&) = delete;

  // Implementation data.
  std::vector<std::string> mru_lists[MRU_LEN];
};

/*  MAHLEN, put this in the CPP that loads all of the models!!
// Example usage in main.cpp
int main() {
  
    // Access the singleton instance through the static getInstance() function
    PluginConfig& instance1 = PluginConfig::getInstance();
    instance1.showMessage("Hello from instance 1");

    // Attempting to get another instance actually returns the same object
    PluginConfig& instance2 = PluginConfig::getInstance();
    instance2.showMessage("Hello from instance 2 (same instance)");

    // Verify that both references point to the same memory address
    if (&instance1 == &instance2) {
        std::cout << "Both instances refer to the same Singleton object!" << std::endl;
    }

    // This would cause a compilation error because the constructor is private:
    // Singleton instance3; 

    // This would also cause a compilation error because the copy constructor is deleted:
    // Singleton instance4 = instance1;

    return 0;
}
*/
} // namespace StochasticTelegraph