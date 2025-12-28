#include "StochasticTelegraph.hpp"

namespace StochasticTelegraph {

static std::string stochasticTelegraphSettingsFileName = asset::user("StochasticTelegraph.json");

bool loadMRUList(MRUList* list){
  FILE *file = fopen(stochasticTelegraphSettingsFileName.c_str(), "r");
  if (file) {
    json_error_t error;
    json_t *rootJ = json_loadf(file, 0, &error);
    std::string mru_name = "mru_" + std::to_string(list->type);
    json_t *mruJ = json_object_get(rootJ, mru_name.c_str());
    if (mruJ) {
      size_t index;
      json_t *value;  
      json_array_foreach(mruJ, index, value) {
        list->entries.push_back(json_string_value(value));
      }
    }
    fclose(file);
    json_decref(rootJ);
    return true;
  } else {
    return false; 
  }
}

void saveMRUList(const MRUList list){
  FILE *file = fopen(stochasticTelegraphSettingsFileName.c_str(), "r");
  json_t *rootJ = nullptr;
  if (file) {
    json_error_t error;
    rootJ = json_loadf(file, 0, &error);
    fclose(file);
  }
  if (!rootJ) {
    rootJ = json_object();
  }
  std::string mru_name = "mru_" + std::to_string(list.type);
  json_t *mruJ = json_array();
  for (const auto &item : list.entries) {
    json_array_append_new(mruJ, json_string(item.c_str()));
  }
  json_object_set_new(rootJ, mru_name.c_str(), mruJ);
  file = fopen(stochasticTelegraphSettingsFileName.c_str(), "w");
  if (file) {
    json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_PRESERVE_ORDER);
    fclose(file);
  }
  json_decref(rootJ);
}

} // namespace StochasticTelegraph