#include "StochasticTelegraph.hpp"

namespace StochasticTelegraph {

const size_t MAX_MRU_DIRECTORIES = 10;

static std::string stochasticTelegraphSettingsFileName = asset::user("StochasticTelegraph.json");

PluginConfig::PluginConfig() {
  // Load from JSON.
  FILE *file = fopen(stochasticTelegraphSettingsFileName.c_str(), "r");
  json_t *rootJ = nullptr;
  if (file) {
    json_error_t error;
    rootJ = json_loadf(file, 0, &error);
    fclose(file);
  }
  if (!rootJ) {
    // No config found. Leave data empty.
    WARN("PluginConfig constructor called, but no config found.");
    return;
  }
  for (int i = 0; i < MRU_LEN; i++) {
    std::string mru_name = "mru_" + std::to_string(i);
    json_t *mruJ = json_object_get(rootJ, mru_name.c_str());
    if (mruJ) {
      size_t index;
      json_t *value;  
      json_array_foreach(mruJ, index, value) {
        mru_lists[i].push_back(json_string_value(value));
      }
    }
  }
  json_decref(rootJ);
}

void PluginConfig::saveConfigToDisk() {
  FILE *file = fopen(stochasticTelegraphSettingsFileName.c_str(), "w");
  if (file) {
    json_t *rootJ = json_object();
    for (int i = 0; i < MRU_LEN; i++) {
      if (mru_lists[i].empty()) {
        continue;
      }
      std::string mru_name = "mru_" + std::to_string(i);
      json_t *mruJ = json_array();
      for (const auto &item : mru_lists[i]) {
        json_array_append_new(mruJ, json_string(item.c_str()));
      }
      json_object_set_new(rootJ, mru_name.c_str(), mruJ);
    }
    json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_PRESERVE_ORDER);
    fclose(file);
    json_decref(rootJ);
  }
}

std::vector<std::string>* PluginConfig::getMRUList(MRUType type) {
  return &mru_lists[type];
}

void PluginConfig::putMRUEntry(const std::string& entry, MRUType type) {
  // Remove the entry if it already exists
  auto& current_list = mru_lists[type];
  // See if load_folder is already in the list.
  auto it = std::find(current_list.begin(), current_list.end(), entry);
  
  // Some small optimizations to avoid unneeded saving.
  bool must_save = false;
  if (it != current_list.end()) {
    if (it != current_list.begin()) {
      // It is already in the list but not at the front; move it to the front.
      current_list.erase(it);
      // Add the entry to the front
      current_list.insert(current_list.begin(), entry);
      must_save = true;
    } // If it is already at the front, do nothing.
  } else {
    // Add the entry to the front
    current_list.insert(current_list.begin(), entry);
    must_save = true;
  }

  if (!must_save) {
    return;  // No change needed.
  }
  // Trim to max size.
  if (current_list.size() > MAX_MRU_DIRECTORIES) {
    current_list.resize(MAX_MRU_DIRECTORIES);
  }
  saveConfigToDisk();
}

} // namespace StochasticTelegraph