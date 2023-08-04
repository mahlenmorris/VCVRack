#include <algorithm>
#include <cctype>
#include <cmath>
#include <unordered_map>
#include <utility>  // pair
#include <vector>

#include "plugin.hpp"
#include "st_textfield.hpp"
#include "tipsy/tipsy.h"

struct TTY : Module {
  static const int DEFAULT_WIDTH = 18;
  enum ParamId {
		CLEAR_PARAM,
    SAMPLE_PARAM,
    PAUSE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		V1_INPUT,
    TEXT1_INPUT,
    V2_INPUT,
    TEXT2_INPUT,
		TEXT3_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUTS_LEN
	};
	enum LightId {
    CLEAR_LIGHT,
    PAUSE_LIGHT,
		LIGHTS_LEN
	};

  TTY() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch(PAUSE_PARAM, 0, 1, 0, "Stops changing the screen contents",
                 {"Writing", "Paused"});
		configParam(CLEAR_PARAM, 0.f, 1.f, 0.f, "Clears all output");
    configParam(SAMPLE_PARAM, 1000, 0, 50, "Number of samples skipped between logging attempts");
    getParamQuantity(SAMPLE_PARAM)->snapEnabled = true;
		configInput(V1_INPUT, "New values will be shown on screen");
    configInput(V2_INPUT, "New values will be shown on screen");
    configInput(TEXT1_INPUT, "Input for Tipsy text info");
    configInput(TEXT2_INPUT, "Input for Tipsy text info");
    configInput(TEXT3_INPUT, "Input for Tipsy text info");
    decoder1.provideDataBuffer(recvBuffer1, recvBufferSize);
    decoder2.provideDataBuffer(recvBuffer2, recvBufferSize);
    decoder3.provideDataBuffer(recvBuffer3, recvBufferSize);
  }

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "width", json_integer(width));

    json_object_set_new(rootJ, "screen_colors", json_integer(screen_colors));
    if (font_choice.length() > 0) {
      json_object_set_new(rootJ, "font_choice",
                          json_stringn(font_choice.c_str(), font_choice.size()));
    }
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
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

  void add_string(const std::string &next_string) {
    // We add to the queue, but we limit how large the pending queue
    // can get.
    if (additions.text_additions.size() < additions.text_additions.max_size()) {
      additions.text_additions.push(next_string);
    }
  }

  void process(const ProcessArgs& args) override {
    bool paused = params[PAUSE_PARAM].getValue() > 0;

    bool clear_command_received = false;
    // Unpausing puts the cursor at the end of the text.
    // If we don't then text gets added but scrolls off the screen.
    if (!paused && was_paused) {
      // Setting to outlandishly high value; will be changed to "end-of-file"
      // by TTYTextField.
      cursor_override = 2000000;
    }

    if (!paused) {
      tick_count += 1;
      // ">" is correct here. The SAMPLE_PARAM is really the number of samples
      // to skip, and can go to zero.
      if (tick_count > params[SAMPLE_PARAM].getValue()) {
        tick_count = 0;
        if (inputs[V1_INPUT].isConnected()) {
          float v1 = inputs[V1_INPUT].getVoltage();
          if (v1 != previous_v1) {  // TODO: use instead the minimum distance calculation.
            previous_v1 = v1;
            std::string str_value = std::to_string(v1);
            // Hmmmm; should I be comparing the string values instead? It would be
            // odd to see the same string twice on a tighly changing value....

            // Add to buffer.
            std::string next(str_value);
            next.append("\n");
            add_string(next);
          }
        }
        if (inputs[V2_INPUT].isConnected()) {
          float v2 = inputs[V2_INPUT].getVoltage();
          if (v2 != previous_v2) {  // TODO: use instead the minimum distance calculation.
            previous_v2 = v2;
            std::string str_value = std::to_string(v2);
            // Hmmmm; should I be comparing the string values instead? It would be
            // odd to see the same string twice on a tighly changing value....

            // Add to buffer.
            std::string next(str_value);
            next.append("\n");
            add_string(next);
          }
        }
      }
    }

    if (inputs[TEXT1_INPUT].isConnected()) {
      auto decoder_status = decoder1.readFloat(inputs[TEXT1_INPUT].getVoltage());
      // Condense all of this into a class.
      if (!paused) {
        if (!decoder1.isError(decoder_status)) {
          if (decoder_status == tipsy::DecoderResult::BODY_READY) {
            // TODO: Obviously check more things like that it's a string type.
            std::string next(std::string((const char *) recvBuffer1));
            if (next.compare("!!CLEAR!!") == 0) {
              clear_command_received = true;
              clear_light_countdown = std::floor(args.sampleRate / 10.0f);
            } else {
              next.append("\n");
              add_string(next);
            }
          }
        }
      }
    }
    if (inputs[TEXT2_INPUT].isConnected()) {
      auto decoder_status = decoder2.readFloat(inputs[TEXT2_INPUT].getVoltage());
      if (!paused) {
        if (!decoder2.isError(decoder_status)) {
          if (decoder_status == tipsy::DecoderResult::BODY_READY) {
            // TODO: Obviously check more things like that it's a string type.
            std::string next(std::string((const char *) recvBuffer2));
            if (next.compare("!!CLEAR!!") == 0) {
              clear_command_received = true;
              clear_light_countdown = std::floor(args.sampleRate / 10.0f);
            } else {
              next.append("\n");
              add_string(next);
            }
          }
        }
      }
    }
    if (inputs[TEXT3_INPUT].isConnected()) {
      auto decoder_status = decoder3.readFloat(inputs[TEXT3_INPUT].getVoltage());
      if (!paused) {
        if (!decoder3.isError(decoder_status)) {
          if (decoder_status == tipsy::DecoderResult::BODY_READY) {
            // TODO: Obviously check more things like that it's a string type.
            std::string next(std::string((const char *) recvBuffer3));
            if (next.compare("!!CLEAR!!") == 0) {
              clear_command_received = true;
              clear_light_countdown = std::floor(args.sampleRate / 10.0f);
            } else {
              next.append("\n");
              add_string(next);
            }
          }
        }
      }
    }
    was_paused = paused;
    // Buttons.
    // Note that we don't bother to set clear_light_countdown when the user
    // presses the button; we just light up the button while it's
    // being pressed.
    // We only STOP clearing when the button is released.
    bool clear = clear_command_received || params[CLEAR_PARAM].getValue() > 0.1f;

    if (clear) {
      text.clear();
    }

    // Lights.
    if (clear_light_countdown > 0) {
      clear_light_countdown--;
    }
    lights[CLEAR_LIGHT].setBrightness(
      clear_light_countdown > 0 || clear ? 1.0f : 0.0f);
    lights[PAUSE_LIGHT].setBrightness(paused);
  }

  static constexpr size_t recvBufferSize{1024 * 64};
  unsigned char recvBuffer1[recvBufferSize];
  tipsy::ProtocolDecoder decoder1;
  unsigned char recvBuffer2[recvBufferSize];
  tipsy::ProtocolDecoder decoder2;
  unsigned char recvBuffer3[recvBufferSize];
  tipsy::ProtocolDecoder decoder3;

  int clear_light_countdown = 0;

  // If we transition from paused -> flowing, move the cursor to the bottom
  // of the text.
  bool was_paused = false;

  // Don't only update every single tick. It spews out too many rows too quickly.
  // We let user control frequency. Small values let you see the Tipsy protocol
  // and find odd transients, larger values let you see the movement in
  // fewer lines.
  int tick_count = 0;

  float previous_v1 = -1000.29349;  // Some value it won't be.
  float previous_v2 = -1000.29349;  // Some value it won't be.

  // Controls.

  bool editor_refresh = false;
  ///////
  // UI related
  // Full text of output. Also used by TTYTextField for viewing.
  std::string text;
  // Text to be added to the bottom of the screen.
  TTYQueue additions;

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
  int old_width, new_width;
  // Having left-side resize means the 'box' for the module can move.
  float old_posx, new_posx;

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
      module->width = this->old_width;
      module->box_pos_x = this->old_posx;  // Used by TTYWidget::step.
      module->update_pos = true;
    }
  }

  void redo() override {
    TTY *module = dynamic_cast<TTY*>(APP->engine->getModule(moduleId));
    if (module) {
      module->width = this->new_width;
      module->box_pos_x = this->new_posx;
      module->update_pos = true;
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
		const float minWidth = 4 * RACK_GRID_WIDTH;
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
        cursor = std::min(module->cursor_override, (int) module->text.size());
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
    if (module) {
      if (module->additions.text_additions.size() > 0) {
        make_additions(&(module->additions));
        module->editor_refresh = true;
      }
      if (color_scheme != module->screen_colors || module->editor_refresh) {
        // Note: this doesn't actully care about editor_refresh. But this cleared
        // up a bug about duplicated windows not keeping the same color.
        color_scheme = module->screen_colors;
        color = int_to_color(color_scheme >> 24);
        bgColor = int_to_color(color_scheme & 0xffffff);
      }
  		if (module->editor_refresh) {
        // TODO: is this checked often enough? I don't know when step()
        // is called.
        // Text has been changed, editor needs to update itself.
        // This happens when the module loads, and on undo/redo.
  			textUpdated();
  			module->editor_refresh = false;
  		}
    }
	}
};

static std::string module_browser_text =
  "TODO: write this text\n";

struct TTYDisplay : LedDisplay {
  TTYTextField* textField;

	void setModule(TTY* module) {
		textField = createWidget<TTYTextField>(Vec(0, 0));
    textField->allow_text_entry = false;  // Don't let user type text here.
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
    if (textField->module && textField->module->width <= 4) {
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

    addParam(createParamCentered<RoundBlackKnob>(
         mm2px(Vec(8.938, 46.0)), module, TTY::SAMPLE_PARAM));
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(8.938, 59.0)),
                                             module, TTY::PAUSE_PARAM,
                                             TTY::PAUSE_LIGHT));
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(8.938, 71.0)),
                                             module, TTY::CLEAR_PARAM,
                                             TTY::CLEAR_LIGHT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.938, 16.0)), module,
        TTY::V1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.938, 29.0)), module,
        TTY::V2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.938, 86.0)), module,
        TTY::TEXT1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.938, 101.872)), module,
        TTY::TEXT2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.938, 118.579)), module,
        TTY::TEXT3_INPUT));

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
