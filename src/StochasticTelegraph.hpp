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

/////////////////////
// Widget for resizing certain modules.
// T is module type, U is *UndoRedo type.
template <typename T, typename U>
struct STResizeHandle : OpaqueWidget {
  Vec dragPos;
  Rect originalBox;
  T* module;
  bool right;  // True for one on the right side.
  // Limits on size.
  int min_holes, max_holes;

  STResizeHandle(bool is_right, int min_h, int max_h) : 
      right(is_right), min_holes(min_h), max_holes(max_h) {
    // One hole wide and full length tall.
    box.size = Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
  }

  void onDragStart(const DragStartEvent& e) override {
    if (e.button != GLFW_MOUSE_BUTTON_LEFT)
      return;

    dragPos = APP->scene->rack->getMousePos();
    ModuleWidget* mw = getAncestorOfType<ModuleWidget>();
    assert(mw);
    originalBox = mw->box;
  }

  void onDragMove(const DragMoveEvent& e) override {
    ModuleWidget* mw = getAncestorOfType<ModuleWidget>();
    assert(mw);
    int original_width = module->width;

    Vec newDragPos = APP->scene->rack->getMousePos();
    float deltaX = newDragPos.x - dragPos.x;

    Rect newBox = originalBox;
    Rect oldBox = mw->box;
    // Minimum and maximum number of holes we allow the module to be.
    const float minWidth = min_holes * RACK_GRID_WIDTH;
    const float maxWidth = max_holes * RACK_GRID_WIDTH;
    if (right) {
      newBox.size.x += deltaX;
      newBox.size.x = std::fmax(newBox.size.x, minWidth);
      newBox.size.x = std::fmin(newBox.size.x, maxWidth);
      newBox.size.x = std::round(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
    } else {
      newBox.size.x -= deltaX;
      newBox.size.x = std::fmax(newBox.size.x, minWidth);
      newBox.size.x = std::fmin(newBox.size.x, maxWidth);
      newBox.size.x = std::round(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
      newBox.pos.x = originalBox.pos.x + originalBox.size.x - newBox.size.x;
    }
    // Set box and test whether it's valid.
    mw->box = newBox;
    if (!APP->scene->rack->requestModulePos(mw, newBox.pos)) {
      mw->box = oldBox;
    }
    module->width = std::round(mw->box.size.x / RACK_GRID_WIDTH);
    if (original_width != module->width) {
      // Make this resizing an undo/redo action. If I don't do this, undoing a
      // different module's move may cause them to overlap (aka, a
      // transporter malfunction).
      APP->history->push(
        new U(module->id, original_width, module->width,
                oldBox.pos.x, mw->box.pos.x));
      // Also need to tell FramebufferWidget to update the appearance.
      module->RedrawText();
    }
  }

  void drawLayer(const DrawArgs& args, int layer) override {
    if (layer == 1) {
      // Draw two lines to give people something to grab for.
      // Lifted from the VCV Blank module.
      for (float x = 5.0; x <= 10.0; x += 5.0) {
        nvgBeginPath(args.vg);
        const float margin = 5.0;
        nvgMoveTo(args.vg, x + 0.5, margin + 0.5);
        nvgLineTo(args.vg, x + 0.5, box.size.y - margin + 0.5);
        nvgStrokeWidth(args.vg, 1.0);
        nvgStrokeColor(args.vg, nvgRGBAf(0.5, 0.5, 0.5, 0.5));
        nvgStroke(args.vg);
      }
    }
  }
};

} // namespace StochasticTelegraph