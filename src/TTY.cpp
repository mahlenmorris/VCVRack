#include <algorithm>
#include <cctype>
#include <cmath>
#include <thread>
#include <unordered_map>
#include <utility>  // pair
#include <vector>

#include "plugin.hpp"
#include "st_textfield.hpp"

struct TTY : Module {
  static const int DEFAULT_WIDTH = 18;
  enum ParamId {
		CLEAR_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		V1_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUTS_LEN
	};
	enum LightId {
    CLEAR_LIGHT,
		LIGHTS_LEN
	};

  TTY() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(CLEAR_PARAM, 0.f, 1.f, 0.f, "Clears all output");
		configInput(V1_INPUT, "New values will be shown on screen");
  }

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "text", json_stringn(text.c_str(), text.size()));
    json_object_set_new(rootJ, "width", json_integer(width));

    json_object_set_new(rootJ, "screen_colors", json_integer(screen_colors));
    if (font_choice.length() > 0) {
      json_object_set_new(rootJ, "font_choice",
                          json_stringn(font_choice.c_str(), font_choice.size()));
    }
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* textJ = json_object_get(rootJ, "text");
		if (textJ) {
			text = json_string_value(textJ);
      previous_text = text;
  		editor_refresh = true;
    }
    json_t* widthJ = json_object_get(rootJ, "width");
		if (widthJ)
			width = json_integer_value(widthJ);
      json_t* screenJ = json_object_get(rootJ, "screen_colors");
  		if (screenJ)
  			screen_colors = json_integer_value(screenJ);
    json_t* font_choiceJ = json_object_get(rootJ, "font_choice");
		if (font_choiceJ) {
			font_choice = json_string_value(font_choiceJ);
    }
  }

  std::string getFontPath() {
    // font_choice changes very rarely; cache this value?
    if (font_choice.substr(0, 4) == "res/") {
      return asset::system(font_choice);
    } else {
      return asset::plugin(pluginInstance, font_choice);
    }
  }

  void process(const ProcessArgs& args) override {
  }

  bool editor_refresh = false;
  ///////
  // UI related
  // Full text of program.  Also used by TTYTextField for editing.
  std::string text;
  // We need to the immediately previous version of the text around to
  // make undo and redo work; otherwise, we don't know what the change was.
  std::string previous_text;
  // We want to track the previous placement of the cursor, so that an
  // undo can take us back there.
  int previous_cursor = 0;
  // Amber on Black is the default; it matches the !!!! decor perfectly.
  long long int screen_colors = 0xffc000000000;
  // width (in "holes") of the whole module. Changed by the resize bar on the
  // right (within limits), and informs the size of the display and text field.
  // Saved in the json for the module.
  int width = TTY::DEFAULT_WIDTH;
  // The undo/redo sometimes needs to reset the cursor position.
  // But we don't actully have a good pointer to the text field.
  // drawLayer() uses this if it's > -1.
  int cursor_override = -1;
  // The undo/redo sometimes needs to reset the module position.
  // But we don't actully have a good pointer to the TTYWidget.
  // TTYWidget::step() uses this if update_pos is set.
  float box_pos_x;
  bool update_pos = false;
  // Can be overriden by saved menu choice.
  std::string font_choice = "fonts/RobotoSlab-Regular.ttf";
};

// Adds support for undo/redo in the text field where people type programs.
struct TTYUndoRedoAction : history::ModuleAction {
  std::string old_text;
  std::string new_text;
  int old_cursor, new_cursor;
  int old_width, new_width;
  // Having left-side resize means the 'box' for the module can move.
  float old_posx, new_posx;

  TTYUndoRedoAction(int64_t id, std::string oldText, std::string newText,
     int old_cursor_pos, int new_cursor_pos) : old_text{oldText},
         new_text{newText}, old_cursor{old_cursor_pos},
         new_cursor{new_cursor_pos} {
    moduleId = id;
    name = "text edit";
    old_width = new_width = -1;
  }
  TTYUndoRedoAction(int64_t id, int old_width, int new_width,
                        float old_posx, float new_posx) :
      old_width{old_width}, new_width{new_width}, old_posx{old_posx},
      new_posx{new_posx} {
    moduleId = id;
    name = "module width change";
  }
  void undo() override {
    TTY *module = dynamic_cast<TTY*>(APP->engine->getModule(moduleId));
    if (module) {
      if (old_width < 0) {
        module->text = this->old_text;
        // Tell UI it needs to refresh because 'text' has changed.
        module->editor_refresh = true;
        module->cursor_override = old_cursor;
      } else {
        module->width = this->old_width;
        module->box_pos_x = this->old_posx;  // Used by TTYWidget::step.
        module->update_pos = true;
      }
    }
  }

  void redo() override {
    TTY *module = dynamic_cast<TTY*>(APP->engine->getModule(moduleId));
    if (module) {
      if (old_width < 0) {
        module->text = this->new_text;
        // Tell UI it needs to refresh because 'text' has changed.
        module->editor_refresh = true;
        module->cursor_override = new_cursor;
      } else {
        module->width = this->new_width;
        module->box_pos_x = this->new_posx;
        module->update_pos = true;
      }
    }
  }
};

// Needs to have a different name than the one in BASICally, or it
// compiles but just won't work.
// TODO: Possible to make this a reusable class between the two? Unlikely.
struct TTYModuleResizeHandle : OpaqueWidget {
	Vec dragPos;
	Rect originalBox;
	TTY* module;
  bool right = false;  // True for one on the right side.

	TTYModuleResizeHandle() {
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
		const float minWidth = 3 * RACK_GRID_WIDTH;
    const float maxWidth = 64 * RACK_GRID_WIDTH;
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
      // Make resizing an undo/redo action. If I don't do this, undoing a
      // different module's move will cause them to overlap (aka, a
      // transporter malfunction).
      APP->history->push(
        new TTYUndoRedoAction(module->id, original_width, module->width,
                                  oldBox.pos.x, mw->box.pos.x));
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

// Class for the editor.
struct TTYTextField : STTextField {
	TTY* module;
  long long int color_scheme;

  NVGcolor int_to_color(int color) {
    return nvgRGB(color >> 16, (color & 0xff00) >> 8, color & 0xff);
  }

  // bgColor seems to have no effect if I don't do this. Drawing a background
  // and then letting LedDisplayTextField draw the rest will fixes that.
  void drawLayer(const DrawArgs& args, int layer) override {
    nvgScissor(args.vg, RECT_ARGS(args.clipBox));

    if (layer == 1) {
  		// background only
      nvgBeginPath(args.vg);
      nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
      nvgFillColor(args.vg, bgColor);
      nvgFill(args.vg);

      if (module && module->cursor_override >= 0) {
        // Undo/redo must have just happened.
        // Move cursor (with no selection) to where the cursor was when we
        // did edit.
        cursor = module->cursor_override;
        selection = module->cursor_override;
        module->cursor_override = -1;
        // Since we just forcibly moved the cursor, need to reposition window
        // to show it.
        extended.RepositionWindow(cursor);
      }
  	}
  	STTextField::drawLayer(args, layer);  // Draw text.
  	nvgResetScissor(args.vg);
  }

	void step() override {
		STTextField::step();
    if (module && (color_scheme != module->screen_colors ||
                   module->editor_refresh)) {
      // Note: this doesn't actully care about editor_refresh. But this cleared
      // up a bug about duplicated windows not keeping the same color.
      color_scheme = module->screen_colors;
      color = int_to_color(color_scheme >> 24);
      bgColor = int_to_color(color_scheme & 0xffffff);
    }
		if (module && module->editor_refresh) {
      // TODO: is this checked often enough? I don't know when step()
      // is called.
      // Text has been changed, editor needs to update itself.
      // This happens when the module loads, and on undo/redo.
			textUpdated();
			module->editor_refresh = false;
		}
	}

  // User has updated the text.
	void onChange(const ChangeEvent& e) override {
		if (module) {
      // Sometimes the text isn't actually different. If I don't check
      // this, I might get spurious history events.
      // TODO: do I need this check anymore?
      if (module->text != module->previous_text) {
        APP->history->push(
          new TTYUndoRedoAction(module->id, module->previous_text,
                             module->text, module->previous_cursor, cursor));
        module->previous_text = module->text;
      }
      module->previous_cursor = cursor;
    }
	}
};

static std::string module_browser_text =
  "TODO: write this text\n";

struct TTYDisplay : LedDisplay {
  TTYTextField* textField;

	void setModule(TTY* module) {
		textField = createWidget<TTYTextField>(Vec(0, 0));
		textField->box.size = box.size;
		textField->module = module;
    // If this is the module browser, 'module' will be null!
    if (module != nullptr) {
      textField->text = &(module->text);
    } else {
      // Show something inviting when being shown in the module browser.
      textField->text = &module_browser_text;
    }
    textField->textUpdated();
		addChild(textField);
	}

  // The TTYWidget changes size, so we have to reflect that.
  void step() override {
    // At smaller sizes, hide the screen.
    if (textField->module && textField->module->width <= 8) {
      hide();
    } else {
      show();
    }
    textField->box.size = box.size;
    LedDisplay::step();
	}

  // Text insertions from the menu.
  void insertText(const std::string &fragment) {
    textField->insertText(fragment);
  }

  // Setting the font.
  void setFontPath() {
    if (textField && textField->module) {
      textField->fontPath = textField->module->getFontPath();
    }
  }
};

const float NON_SCREEN_WIDTH = 2.0f;
const float CONTROL_WIDTH = 13.0f;

struct TTYWidget : ModuleWidget {
  Widget* topRightScrew;
	Widget* bottomRightScrew;
	TTYModuleResizeHandle* rightHandle;
	TTYDisplay* textDisplay;

  TTYWidget(TTY* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/TTY.svg")));

    // Set reasonable initial size of module. Will likely get updated below.
    box.size = Vec(RACK_GRID_WIDTH * TTY::DEFAULT_WIDTH, RACK_GRID_HEIGHT);
		if (module) {
      // Set box width from loaded Module when available.
			box.size.x = module->width * RACK_GRID_WIDTH;
    } else {
      // Like when showing the module in the module browser.
      box.size.x = TTY::DEFAULT_WIDTH * RACK_GRID_WIDTH;
    }

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    topRightScrew = createWidget<ScrewSilver>(
        Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0));
    addChild(topRightScrew);
    // TODO: this next line's Y coordinate is very odd.
    addChild(createWidget<ScrewSilver>(
        Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    bottomRightScrew = createWidget<ScrewSilver>(
        Vec(box.size.x - 2 * RACK_GRID_WIDTH,
            RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
    addChild(bottomRightScrew);

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.938, 59.922)), module, TTY::V1_INPUT));
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(8.938, 81.303)),
                                             module, TTY::CLEAR_PARAM,
                                             TTY::CLEAR_LIGHT));

    textDisplay = createWidget<TTYDisplay>(mm2px(Vec(18.08, 5.9)));
		textDisplay->box.size = mm2px(Vec(60.0, 117.0));
    textDisplay->box.size.x = box.size.x - RACK_GRID_WIDTH * NON_SCREEN_WIDTH - mm2px(CONTROL_WIDTH);
		textDisplay->setModule(module);

		addChild(textDisplay);

    // Resize bar on right.
    rightHandle = new TTYModuleResizeHandle;
    rightHandle->right = true;
		rightHandle->module = module;
    // Make sure the handle is correctly placed if drawing for the module
    // browser.
    rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;
    addChild(rightHandle);

    // Update the font in the code window to be the one chosen in the menu.
    textDisplay->setFontPath();
  }

  void step() override {
		TTY* module = dynamic_cast<TTY*>(this->module);
    // While this is really only useful to call when the width changes,
    // I don't think it's currently worth the effort to ONLY call it then.
    // And maybe the *first* time step() is called.
		if (module) {
			box.size.x = module->width * RACK_GRID_WIDTH;
      // The right-hand screws have slightly different logic.
      if (module->width < 8) {
        topRightScrew->hide();
      	bottomRightScrew->hide();
      } else {
        topRightScrew->show();
        bottomRightScrew->show();
      }
      if (module->update_pos) {
        module->update_pos = false;
        box.pos.x = module->box_pos_x;
      }
		} else {
      // Like when showing the module in the module browser.
      box.size.x = TTY::DEFAULT_WIDTH * RACK_GRID_WIDTH;
    }
    // Adjust size of area we display text in; it's a function of the size
    // of the module minus some set width.
		textDisplay->box.size.x = box.size.x - RACK_GRID_WIDTH * NON_SCREEN_WIDTH - mm2px(CONTROL_WIDTH);
    // Move the right side screws to follow.
		topRightScrew->box.pos.x = box.size.x - 30;
		bottomRightScrew->box.pos.x = box.size.x - 30;
		rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;

		ModuleWidget::step();
	}

  void appendContextMenu(Menu* menu) override {
    TTY* module = dynamic_cast<TTY*>(this->module);
    // Add color choices.
    menu->addChild(new MenuSeparator);
    std::pair<std::string, long long int> colors[] = {
      {"Green on Black", 0x00ff00000000},
      {"White on Black", 0xffffff000000},
      {"Yellow on Black (like Notes)", 0xffd714000000},
      {"Amber on Black", 0xffc000000000},
      {"Blue on Black", 0x29b2ef000000},
      {"Black on White", 0x000000ffffff},
      {"Blue on White", 0x29b2efffffff}
    };
    MenuItem* color_menu = createSubmenuItem("Screen Colors", "",
     [=](Menu* menu) {
         for (auto line : colors) {
           menu->addChild(createCheckMenuItem(line.first, "",
           [=]() {return line.second == module->screen_colors;},
           [=]() {module->screen_colors = line.second;}
           ));
         }
     }
    );
    menu->addChild(color_menu);

    std::pair<std::string, std::string> fonts[] = {
      {"VCV font (like Notes)", "res/fonts/ShareTechMono-Regular.ttf"},
      {"RobotoMono Bold", "fonts/RobotoMono-Bold.ttf"},
      {"RobotoMono Light", "fonts/RobotoMono-Light.ttf"},
      {"RobotoMono Medium", "fonts/RobotoMono-Medium.ttf"},
      {"RobotoMono Regular", "fonts/RobotoMono-Regular.ttf"},
      {"RobotoSlab Bold", "fonts/RobotoSlab-Bold.ttf"},
      {"RobotoSlab Light", "fonts/RobotoSlab-Light.ttf"},
      {"RobotoSlab Regular", "fonts/RobotoSlab-Regular.ttf"}
  };

    MenuItem* font_menu = createSubmenuItem("Font", "",
      [=](Menu* menu) {
          for (auto line : fonts) {
            menu->addChild(createCheckMenuItem(line.first, "",
                [=]() {return line.second == module->font_choice;},
                [=]() {module->font_choice = line.second;
                       textDisplay->setFontPath();}
            ));
          }
      }
    );
    menu->addChild(font_menu);

  }
};

Model* modelTTY = createModel<TTY, TTYWidget>("TTY");
