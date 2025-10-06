#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
  pluginInstance = p;

  // Add modules here
  p->addModel(modelBasically);
  p->addModel(modelBrainwash);
  p->addModel(modelDepict);
  p->addModel(modelDrifter);
  p->addModel(modelEmbellish);
  p->addModel(modelFermata);
  p->addModel(modelFixation);
  p->addModel(modelFuse);
  p->addModel(modelMemory);
  p->addModel(modelMemoryCV);
  p->addModel(modelRuminate);
  p->addModel(modelTTY);
  p->addModel(modelVenn);

  // Any other plugin initialization may go here.
  // As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
