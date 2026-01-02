#include <algorithm>
#include <cctype>
#include <cmath>
#include <unordered_map>
#include <utility>  // pair
#include <vector>

#include "plugin.hpp"
#include "st_textfield.hpp"
#include "StochasticTelegraph.hpp"

struct Fermata : Module {
  static const int DEFAULT_WIDTH = 18;
  // Many actions mean we need to force the buffer to recalculate the
  // text appearance, so we keep the FramebufferWidget available.
  FramebufferWidget* main_text_framebuffer = nullptr;

  Fermata() {
  }

  void RedrawText() {
    if (main_text_framebuffer != nullptr) {
      main_text_framebuffer->setDirty();
    }
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
    json_object_set_new(rootJ, "visible_lines", json_integer(visible_lines));
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
    json_t* visible_linesJ = json_object_get(rootJ, "visible_lines");
    if (visible_linesJ)
        visible_lines = json_integer_value(visible_linesJ);
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
  int visible_lines = 28;
  // width (in "holes") of the whole module. Changed by the resize bar on the
  // right (within limits), and informs the size of the display and text field.
  // Saved in the json for the module.
  int width = Fermata::DEFAULT_WIDTH;
  // Visible title for the text, set in the menu.
  std::string title_text;
  // The undo/redo sometimes needs to reset the cursor position.
  // But we don't actully have a good pointer to the text field.
  // drawLayer() uses this if it's > -1.
  int cursor_override = -1;
  // The undo/redo sometimes needs to reset the module position.
  // But we don't actully have a good pointer to the FermataWidget.
  // FermataWidget::step() uses this if update_pos is set.
  float box_pos_x;
  bool update_pos = false;
  // Can be overriden by saved menu choice.
  std::string font_choice = "fonts/RobotoSlab-Regular.ttf";
};

// Adds support for undo/redo in the text field where people type programs.
struct FermataUndoRedoAction : history::ModuleAction {
  std::string old_text;
  std::string new_text;
  int old_cursor, new_cursor;
  int old_width, new_width;
  // Having left-side resize means the 'box' for the module can move.
  float old_posx, new_posx;

  FermataUndoRedoAction(int64_t id, std::string oldText, std::string newText,
     int old_cursor_pos, int new_cursor_pos) : old_text{oldText},
         new_text{newText}, old_cursor{old_cursor_pos},
         new_cursor{new_cursor_pos} {
    moduleId = id;
    name = "text edit";
    old_width = new_width = -1;
  }
  FermataUndoRedoAction(int64_t id, int old_width, int new_width,
                        float old_posx, float new_posx) :
      old_width{old_width}, new_width{new_width}, old_posx{old_posx},
      new_posx{new_posx} {
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
        module->box_pos_x = this->old_posx;  // Used by FermataWidget::step.
        module->update_pos = true;
      }
      module->RedrawText();
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
        module->box_pos_x = this->new_posx;
        module->update_pos = true;
      }
      module->RedrawText();
    }
  }
};

// The title when shown below the text.
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
        nvgFillColor(args.vg, settings::preferDarkPanels ? color::WHITE :
                                                           color::BLACK);
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
    box.size = Vec(120, 50);
    multiline = false;
  }
};

// The title when the text field is not visible, more prominently on the module.
struct ClosedTitleTextField : LightWidget {
  Fermata* module;

  ClosedTitleTextField() {
    box.size = mm2px(Vec(6 * 5.08, 110));
  }

  void drawLayer(const DrawArgs& args, int layer) override {
    nvgScissor(args.vg, RECT_ARGS(args.clipBox));
    if (layer == 1) {
      // No background color!

      if (module) {
        std::shared_ptr<Font> font = APP->window->loadFont(module->getFontPath());
        if (font) {
          // Width of box needs to respond to width of module.
          int text_holes = module->width - 2;
          box.size = mm2px(Vec(text_holes * 5.08, 110));
          Rect r = box.zeroPos();
          Vec bounding_box = r.getBottomRight();
          std::string text = module->title_text;
          if (text_holes >= 6) {
            // This is when we show the title horizontally, broken into words.
            // We only do this for one size, because titles are usually short
            // [citation needed], and so look pretty good sideways at smaller
            // sizes. But leaving in the code that worked for smaller widths,
            // just in case I want it later.
            // 26 is a good font size for 6 holes of text.
            int max_font_size = floor(26 * text_holes / 6);
            // 21.0 is a good spacing for font size 26.
            float max_spacing = 21.0 * text_holes / 6;

            std::vector<std::string> lines;
            // Break this into words. Since I don't know the font,
            // too much effort to predict length, and don't want to use a
            // TextField.
            // TODO: cache this computation by moving this elsewhere? This
            // section only takes about 5 usec to run.
            auto start = 0U;
            auto end = text.find(' ');
            int longest = 0;
            while (end != std::string::npos) {
              lines.push_back(text.substr(start, end - start));
              longest = std::max(longest, (int) (end - start));
              start = end + 1;
              end = text.find(' ', start);
            }
            std::string last = text.substr(start);
            lines.push_back(last);
            longest = std::max(longest, (int) last.size());
            int font_size = longest < 8 ? max_font_size : floor(max_font_size * 7 / longest);
            float spacing = longest < 8 ? max_spacing : max_spacing * 7 / longest;

            nvgFillColor(args.vg, settings::preferDarkPanels ? color::WHITE :
                                                               color::BLACK);
            nvgFontSize(args.vg, font_size);
            nvgTextAlign(args.vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
            nvgFontFaceId(args.vg, font->handle);
            // Place on the line just off the left edge.
            for (int i = 0; i < (int) lines.size(); i++) {
              nvgText(args.vg, bounding_box.x / 2, i * spacing, lines[i].c_str(), NULL);
            }
          } else {
            // Show it sideways!
            // This is tricky because the font may be taller than the box we're
            // showing the text in, if we just rely on number of holes.
            // So we resize the font to make sure we center and don't clip the
            // top or bottom of the text.
            // One hole appears to be 15mm (i think mm is the units that sizes
            // are given in).
            nvgFontSize(args.vg, 15 * text_holes);
            nvgFontFaceId(args.vg, font->handle);
            nvgTextAlign(args.vg, NVG_ALIGN_LEFT|NVG_ALIGN_BASELINE);
            nvgTextLetterSpacing(args.vg, 0);
            nvgFillColor(args.vg, settings::preferDarkPanels ? color::WHITE :
                                                               color::BLACK);
            float desc, lh;
            nvgTextMetrics(args.vg, NULL, &desc, &lh);
            nvgRotate(args.vg, -M_PI / 2.0f);
            // Because of the rotation, we need to express X, Y as -Y, ???
            nvgText(args.vg, 6 - bounding_box.y, bounding_box.x + desc,
                text.c_str(), NULL);
          }
        }
      }
    }
    LightWidget::drawLayer(args, layer);
    nvgResetScissor(args.vg);
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

// The text that gets show in the module browser and the library.
static std::string module_browser_text =
  "Write your text here! For example:\n"
  "* Instructions for playing the patch.\n"
  "* Notes/reminders on how this part of the patch works.\n"
  "* TODO's, ideas, or a poem you're writing.\n\n"
  "You can also set the title (below) in the module menu, as well as pick "
  "a font, font size, and colors. You can resize the module by dragging the "
  "right edge (over there -->).\n"
  "If you shrink the module enough, the title becomes a large label on "
  "the front.";

// Class for the editor.
struct FermataTextField : STTextField {
  Fermata* module;
  FramebufferWidget* frame_buffer;
  bool was_selected;
  int previous_visible_lines = -1;

  long long int color_scheme;
  std::unordered_map<int, std::pair<int, int>> lines_to_font_size_and_offset;

  FermataTextField() {
    for (int index = 0; index < 13; index++) {
      lines_to_font_size_and_offset.insert({LARGER_TEXT_INFO[index][0],
         std::make_pair(LARGER_TEXT_INFO[index][1], LARGER_TEXT_INFO[index][2])});
    }
  }

  void set_visible_lines(int visible_lines) {
    previous_visible_lines = visible_lines;
    std::unordered_map<int, std::pair<int, int>>::const_iterator found =
       lines_to_font_size_and_offset.find(visible_lines);
    if (found == lines_to_font_size_and_offset.end()) {
      fontSize = 12;
      textOffset = math::Vec(3, 3);
    } else {
      fontSize = found->second.first;
      textOffset = math::Vec(3, (float) (found->second.second));
    }
    // At fontsize 12, it's 28 rows.
    extended.Initialize(visible_lines, visible_lines >= 3 ? 1 : 0);  // Window Size depends on font size.
    textUpdated();
  }

  NVGcolor int_to_color(int color) {
    return nvgRGB(color >> 16, (color & 0xff00) >> 8, color & 0xff);
  }

  // Setting the font. Need to have a method for this, since it can be
  // changed by the menu.
  void setFontPath() {
    if (module) {
      fontPath = module->getFontPath();
    }
  }

  void setModule(Fermata* module, FramebufferWidget* fb_widget) {
    this->module = module;
    frame_buffer = fb_widget;
    // If this is the module browser, 'module' will be null!
    if (module != nullptr) {
      this->text = &(module->text);
      set_visible_lines(module->visible_lines);
    } else {
      // Show something inviting when being shown in the module browser.
      this->text = &module_browser_text;
      set_visible_lines(20);  // Making this larger, so preview is more appealing.
    }
    textUpdated();
  }

  // bgColor seems to have no effect if I don't do this. Drawing a background
  // and then letting STTextField draw the rest fixes that.
  void draw(const DrawArgs& args) override {
    nvgScissor(args.vg, RECT_ARGS(args.clipBox));

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
    STTextField::draw(args);  // Draw text.
    nvgResetScissor(args.vg);
  }

  void step() override {
    // At smaller sizes, hide the screen.
    // User can resize the window, so we need to keep checking for this.
    if (module && module->width <= 8) {
      frame_buffer->hide();
    } else {
      frame_buffer->show();
    }
    if (module && (color_scheme != module->screen_colors ||
                   module->editor_refresh)) {
      // Note: this doesn't actully care about editor_refresh. But this cleared
      // up a bug about duplicated windows not keeping the same color.
      color_scheme = module->screen_colors;
      color = int_to_color(color_scheme >> 24);
      bgColor = int_to_color(color_scheme & 0xffffff);
    }
    if (module && (fabs(previous_visible_lines - module->visible_lines) > 0.1)) {
      set_visible_lines(module->visible_lines);
      frame_buffer->setDirty();
    }
    if (module && module->editor_refresh) {
      // TODO: is this checked often enough? I don't know when step()
      // is called.
      // Text has been changed, editor needs to update itself.
      // This happens when the module loads, and on undo/redo.
      textUpdated();
      frame_buffer->setDirty();
      module->editor_refresh = false;
    }
    STTextField::step();

    bool is_selected = (this == APP->event->selectedWidget);
    if (is_selected != was_selected) {
      was_selected = is_selected;
      is_dirty = true;
    }   
    if (is_dirty) {
      frame_buffer->setDirty();
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
          new FermataUndoRedoAction(module->id, module->previous_text,
                             module->text, module->previous_cursor, cursor));
        module->previous_text = module->text;
      }
      module->previous_cursor = cursor;
    }
    frame_buffer->setDirty();
  }
};

const float NON_SCREEN_WIDTH = 2.0f;
const float NON_TITLE_WIDTH = 4.6f;

struct FermataWidget : ModuleWidget {
  Widget* topRightScrew;
  Widget* bottomRightScrew;
  StochasticTelegraph::STResizeHandle<Fermata, FermataUndoRedoAction>* rightHandle;
  FermataTextField* textField;
  FermataTitleTextField* title;
  ClosedTitleTextField* closed_title;
  FramebufferWidget* main_text_framebuffer;

  FermataWidget(Fermata* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Fermata.svg"),
                         asset::plugin(pluginInstance, "res/Fermata-dark.svg")));

    // Set reasonable initial size of module. Will likely get updated below.
    box.size = Vec(RACK_GRID_WIDTH * Fermata::DEFAULT_WIDTH, RACK_GRID_HEIGHT);
    if (module) {
      // Set box width from loaded Module when available.
      box.size.x = module->width * RACK_GRID_WIDTH;
    } else {
      // Like when showing the module in the module browser.
      box.size.x = Fermata::DEFAULT_WIDTH * RACK_GRID_WIDTH;
    }

    addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
    topRightScrew = createWidget<ThemedScrew>(
        Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0));
    addChild(topRightScrew);
    // TODO: this next line's Y coordinate is very odd.
    addChild(createWidget<ThemedScrew>(
        Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    bottomRightScrew = createWidget<ThemedScrew>(
        Vec(box.size.x - 2 * RACK_GRID_WIDTH,
            RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
    addChild(bottomRightScrew);

    // User created title.
    title = createWidget<FermataTitleTextField>(mm2px(Vec(12.029, 122.3)));
    title->box.size = mm2px(Vec(200.0, 10.0));
    title->module = module;
    addChild(title);

    // User created title when closed.
    closed_title = createWidget<ClosedTitleTextField>(mm2px(Vec(5.08, 15.0)));
    closed_title->module = module;
    closed_title->hide();  // Only shown when at smallest size.
    addChild(closed_title);

    // The FramebufferWidget that caches the appearence of the text, so we
    // don't have to keep redrawing it (and wasting UI CPU to do it).
    main_text_framebuffer = new FramebufferWidget();
    // The actual widget that shows and edits text.
    textField = createWidget<FermataTextField>(
      mm2px(Vec(5.08, 5.9)));  // 5.08 == RACK_GRID_WIDTH in mm.
    textField->box.size = mm2px(Vec(60.0, 117.0));
    textField->box.size.x = box.size.x - RACK_GRID_WIDTH * NON_SCREEN_WIDTH;
    textField->setModule(module, main_text_framebuffer);
    addChild(main_text_framebuffer);
    main_text_framebuffer->addChild(textField);
    if (module) {
      module->main_text_framebuffer = main_text_framebuffer;
    }

    // Resize bar on left.
    StochasticTelegraph::STResizeHandle<Fermata, FermataUndoRedoAction>* leftHandle = 
        new StochasticTelegraph::STResizeHandle<Fermata, FermataUndoRedoAction>(false, 3, 300);
    leftHandle->module = module;
    // Make sure the handle is correctly placed if drawing for the module
    // browser.
    // new_rightHandle->box.pos.x = box.size.x - new_rightHandle->box.size.x;
    addChild(leftHandle);

    // Resize bar on right.
    rightHandle = new StochasticTelegraph::STResizeHandle<Fermata, FermataUndoRedoAction>(true, 3, 300);
    rightHandle->module = module;
    // Make sure the handle is correctly placed if drawing for the module
    // browser.
    rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;
    addChild(rightHandle);

    // Update the font in the code window to be the one chosen in the menu.
    textField->setFontPath();
  }

  void step() override {
    Fermata* module = dynamic_cast<Fermata*>(this->module);
    if (module) {
      if (box.size.x != module->width * RACK_GRID_WIDTH) {
        // Module width has changed. Might be first time step() has been called, or undo/redo has happened
        // But we check for it because when STRIP creates a new instance of this module, it may not
        // pick up the JSON-saved width until after Rack has placed the other modules, causing gaps or
        // overlaps. 
        box.size.x = module->width * RACK_GRID_WIDTH;
        // This forces the other modules to the right place if needed.
        APP->scene->rack->setModulePosForce(this, box.pos);
      }

      // TODO: Should the logic in step() above that shows/hides the text editor also be here?
      if (module->width <= 8) {
        closed_title->show();
        title->hide();
      } else {
        closed_title->hide();
        title->show();
      }
      // The right-hand screws have to disappear when we get thin enough.
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
      box.size.x = Fermata::DEFAULT_WIDTH * RACK_GRID_WIDTH;
    }
    // Adjust size of area we display text in; it's a function of the size
    // of the module minus some set width.
    textField->box.size.x = box.size.x - RACK_GRID_WIDTH * NON_SCREEN_WIDTH;
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
           [=]() {module->screen_colors = line.second;
                  module->RedrawText(); }
           ));
         }
     }
    );
    menu->addChild(color_menu);

    // 28 * (12.0 / fontSize) = rows
    // 12 / fontSize = rows / 28
    // fontSize / 12 = 28 / rows
    // fontSize = 28*12/rows

    MenuItem* visible_lines_menu = createSubmenuItem("Visible Lines", "",
      [=](Menu* menu) {
          for (int index = 0; index < 13; index++) {
            int lines = LARGER_TEXT_INFO[index][0];
            menu->addChild(createCheckMenuItem(std::to_string(lines), "",
                [=]() {return lines == module->visible_lines;},
                [=]() {module->visible_lines = lines;
                       textField->set_visible_lines(module->visible_lines);
                       module->RedrawText();
                      }
            ));
          }
      }
    );
    menu->addChild(visible_lines_menu);


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
                       textField->setFontPath();
                       module->RedrawText(); }
            ));
          }
      }
    );
    menu->addChild(font_menu);

  }
};

Model* modelFermata = createModel<Fermata, FermataWidget>("Fermata");
