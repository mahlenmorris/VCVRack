#include <algorithm>
#include <cctype>
#include <cmath>
#include <thread>
#include <unordered_map>
#include <utility>  // pair
#include <vector>

#include "plugin.hpp"
#include "st_textfield.hpp"

struct Fermata : Module {
  static const int DEFAULT_WIDTH = 18;

  Fermata() {
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
    if (title_text.length() > 0) {
      json_object_set_new(rootJ, "title_text",
                          json_stringn(title_text.c_str(), title_text.size()));
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
    json_t* title_textJ = json_object_get(rootJ, "title_text");
		if (title_textJ) {
			title_text = json_string_value(title_textJ);
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
  // Full text of program.  Also used by FermataTextField for editing.
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
  int width = Fermata::DEFAULT_WIDTH;
  // Visible title for the text, set in the menu.
  std::string title_text;
  // The undo/redo sometimes needs to reset the cursor position.
  // But we don't actully have a good pointer to the text field.
  // drawLayer() uses this if it's > -1;
  int cursor_override = -1;
  // Can be overriden by saved menu choice.
  std::string font_choice = "fonts/RobotoSlab-Regular.ttf";
};

// Adds support for undo/redo in the text field where people type programs.
struct FermataTextEditAction : history::ModuleAction {
  std::string old_text;
  std::string new_text;
  int old_cursor, new_cursor;
  int old_width;
  int new_width;

  FermataTextEditAction(int64_t id, std::string oldText, std::string newText,
     int old_cursor_pos, int new_cursor_pos) : old_text{oldText},
         new_text{newText}, old_cursor{old_cursor_pos},
         new_cursor{new_cursor_pos} {
    moduleId = id;
    name = "text edit";
    old_width = new_width = -1;
  }
  FermataTextEditAction(int64_t id, int old_width, int new_width) :
      old_width{old_width}, new_width{new_width} {
    moduleId = id;
    name = "module width change";
  }
  void undo() override {
    Fermata *module = dynamic_cast<Fermata*>(APP->engine->getModule(moduleId));
    if (module) {
      if (old_width < 0) {
        module->text = this->old_text;
        // Tell UI it needs to refresh because 'text' has changed.
        module->editor_refresh = true;
        module->cursor_override = old_cursor;
      } else {
        module->width = this->old_width;
      }
    }
  }

  void redo() override {
    Fermata *module = dynamic_cast<Fermata*>(APP->engine->getModule(moduleId));
    if (module) {
      if (old_width < 0) {
        module->text = this->new_text;
        // Tell UI it needs to refresh because 'text' has changed.
        module->editor_refresh = true;
        module->cursor_override = new_cursor;
      } else {
        module->width = this->new_width;
      }
    }
  }
};

// Needs to have a different name than the one in BASICally, or it
// compiles but just won't work.
// TODO: Possible to make this a reusable class between the two? Unlikely.
struct FermataModuleResizeHandle : OpaqueWidget {
	Vec dragPos;
	Rect originalBox;
	Fermata* module;

	FermataModuleResizeHandle() {
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
		const float minWidth = 8 * RACK_GRID_WIDTH;
    const float maxWidth = 64 * RACK_GRID_WIDTH;
		newBox.size.x += deltaX;
		newBox.size.x = std::fmax(newBox.size.x, minWidth);
    newBox.size.x = std::fmin(newBox.size.x, maxWidth);
		newBox.size.x = std::round(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;

		// Set box and test whether it's valid.
		mw->box = newBox;
		if (!APP->scene->rack->requestModulePos(mw, newBox.pos)) {
			mw->box = oldBox;
		}
		module->width = std::round(mw->box.size.x / RACK_GRID_WIDTH);
    if (original_width != module->width) {
      // Make this an undo action. If I don't do this, undoing a different
      // module's move will cause them to overlap.
      APP->history->push(
        new FermataTextEditAction(module->id, original_width, module->width));
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

struct FermataTitleTextField : LightWidget {
  Fermata* module;

  FermataTitleTextField() {
  }

  void drawLayer(const DrawArgs& args, int layer) override {
    nvgScissor(args.vg, RECT_ARGS(args.clipBox));
    if (layer == 1) {
      // No background color!

      std::shared_ptr<Font> font;
      std::string text;
      if (module) {
        font = APP->window->loadFont(module->getFontPath());
        text = module->title_text;
      } else {
        font = APP->window->loadFont(
          asset::plugin(pluginInstance, "fonts/RobotoSlab-Regular.ttf"));
        text = "A Longer Note";
      }
      if (font) {
        nvgFillColor(args.vg, color::BLACK);
        // The longer the text, the smaller the font. 20 is our largest size,
        // and it handles 10 chars of this font. 10 is smallest size, it can
        // handle 25 chars.
        int font_size = 18;
        nvgFontSize(args.vg, font_size);
        nvgTextAlign(args.vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
        nvgFontFaceId(args.vg, font->handle);
        nvgText(args.vg, 0, 0, text.c_str(), NULL);
      }
    }
    Widget::drawLayer(args, layer);
    nvgResetScissor(args.vg);
  }
};

struct FermataTextFieldMenuItem : TextField {
	FermataTextFieldMenuItem() {
    box.size = Vec(120, 20);
    multiline = false;
  }
};

struct FermataProgramNameMenuItem : FermataTextFieldMenuItem {
	Fermata* module;

	FermataProgramNameMenuItem(Fermata* fermata_module) {
    module = fermata_module;
    if (module) {
      text = module->title_text;
    }
  }
	void onChange(const event::Change& e) override {
    FermataTextFieldMenuItem::onChange(e);
    if (module) {
      module->title_text = text;
    }
  }
};

// Class for the editor.
struct FermataTextField : STTextField {
	Fermata* module;
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
          new FermataTextEditAction(module->id, module->previous_text,
                             module->text, module->previous_cursor, cursor));
        module->previous_text = module->text;
      }
      module->previous_cursor = cursor;
    }
	}
};

static std::string module_browser_text =
  "Write your text here! For example:\n"
  "* Instructions for playing the patch.\n"
  "* Notes/reminders on how this part of the patch works.\n"
  "* TODO's or ideas.\n"
  "* A short story you're writing while listening to your patch.\n\n"
  "You can also set the title (below) in the module menu, as well as a pick "
  "a font and screen colors. And you can resize the module by dragging the "
  "right edge (over there -->)";

struct FermataDisplay : LedDisplay {
  FermataTextField* textField;

	void setModule(Fermata* module) {
		textField = createWidget<FermataTextField>(Vec(0, 0));
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

  // The FermataWidget changes size, so we have to reflect that.
  void step() override {
    // At smallest size, hide the screen.
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

const float NON_SCREEN_WIDTH = 1.6f;
const float NON_TITLE_WIDTH = 4.6f;

struct FermataWidget : ModuleWidget {
  Widget* topRightScrew;
	Widget* bottomRightScrew;
	Widget* rightHandle;
	FermataDisplay* textDisplay;
  FermataTitleTextField* title;

  FermataWidget(Fermata* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Fermata.svg")));

    // Set reasonable initial size of module. Will likely get updated below.
    box.size = Vec(RACK_GRID_WIDTH * Fermata::DEFAULT_WIDTH, RACK_GRID_HEIGHT);
		if (module) {
      // Set box width from loaded Module when available.
			box.size.x = module->width * RACK_GRID_WIDTH;
    } else {
      // Like when showing the module in the module browser.
      box.size.x = Fermata::DEFAULT_WIDTH * RACK_GRID_WIDTH;
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

    // User created title.
    title = createWidget<FermataTitleTextField>(mm2px(Vec(12.029, 122.3)));
    title->box.size = mm2px(Vec(200.0, 10.0));
    title->module = module;
    addChild(title);

    textDisplay = createWidget<FermataDisplay>(
      mm2px(Vec(3.0, 5.9)));
		textDisplay->box.size = mm2px(Vec(60.0, 117.0));
    textDisplay->box.size.x = box.size.x - RACK_GRID_WIDTH * NON_SCREEN_WIDTH;
		textDisplay->setModule(module);

		addChild(textDisplay);

    // Resize bar on right.
    FermataModuleResizeHandle* new_rightHandle = new FermataModuleResizeHandle;
		this->rightHandle = new_rightHandle;
		new_rightHandle->module = module;
    // Make sure the handle is correctly placed if drawing for the module
    // browser.
    new_rightHandle->box.pos.x = box.size.x - new_rightHandle->box.size.x;
    addChild(new_rightHandle);

    // Update the font in the code window to be the one chosen in the menu.
    textDisplay->setFontPath();
  }

  void step() override {
		Fermata* module = dynamic_cast<Fermata*>(this->module);
    // While this is really only useful to call when the width changes,
    // I don't think it's currently worth the effort to ONLY call it then.
    // And maybe the *first* time step() is called.
		if (module) {
			box.size.x = module->width * RACK_GRID_WIDTH;
		} else {
      // Like when showing the module in the module browser.
      box.size.x = Fermata::DEFAULT_WIDTH * RACK_GRID_WIDTH;
    }

    // Adjust size of area we display text in.
		textDisplay->box.size.x = box.size.x - RACK_GRID_WIDTH * NON_SCREEN_WIDTH;
    // Adjust size of area we display title in.
		title->box.size.x = box.size.x - RACK_GRID_WIDTH * NON_TITLE_WIDTH;
    // Move the right side screws to follow.
		topRightScrew->box.pos.x = box.size.x - 30;
		bottomRightScrew->box.pos.x = box.size.x - 30;
		rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;

		ModuleWidget::step();
	}

  void appendContextMenu(Menu* menu) override {
    Fermata* module = dynamic_cast<Fermata*>(this->module);
    // Add color choices.
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("Set Title"));
    menu->addChild(new FermataProgramNameMenuItem(module));
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

Model* modelFermata = createModel<Fermata, FermataWidget>("Fermata");
