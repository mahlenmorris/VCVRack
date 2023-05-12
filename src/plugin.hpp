#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file.
extern Model* modelBasically;
extern Model* modelDrifter;
extern Model* modelFermata;
extern Model* modelFuse;
