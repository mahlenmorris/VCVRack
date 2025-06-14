#include <algorithm>
#include <cctype>
#include <cmath>
#include <unordered_map>
#include <utility>  // pair
#include <vector>

#include "plugin.hpp"
#include "st_textfield.hpp"
#include <tipsy/tipsy.h>

/*
  TODO's:
  Try to get the scroll position to not reset to the top when text is deleted.
  And consider making a cursor at the end of the output, to show user that's
  where the next output will be? Maybe not, might not be worth the effort.
  Add new type and color to BASICally and Fermata?
  Add Fermata docs to TTY Examples.
  Chat with paul+paul about documenting Tipsy.
*/

static constexpr size_t recvBufferSize{1024 * 64};

struct TextInput {
  int id;
  std::string preface;
  unsigned char recvBuffer[recvBufferSize];
  tipsy::ProtocolDecoder decoder;

  TextInput() {
    decoder.provideDataBuffer(recvBuffer, recvBufferSize);
  }

  void init(int the_id, const std::string &the_preface) {
    id = the_id;
    preface = the_preface;
  }
};

struct TTY : Module {
  static const int DEFAULT_WIDTH = 12;
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

  enum TimeStamp {
    NONE,
    PATCH_TIME,
    LOCAL_TIME
  } ;

  TTY() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch(PAUSE_PARAM, 0, 1, 0, "Stop writing to output",
                 {"Writing", "Paused"});
    configParam(CLEAR_PARAM, 0.f, 1.f, 0.f, "Clears all output");
    configParam(SAMPLE_PARAM, 1000.0f, 0.0f, 50.0f, "Number of milliseconds skipped between V1/V2 logging attempts");
    configInput(V1_INPUT, "New values will be shown on screen");
    configInput(V2_INPUT, "New values will be shown on screen");
    configInput(TEXT1_INPUT, "Input for Tipsy text info");
    configInput(TEXT2_INPUT, "Input for Tipsy text info");
    configInput(TEXT3_INPUT, "Input for Tipsy text info");
    ti1.init(TEXT1_INPUT, "TEXT1");
    ti2.init(TEXT2_INPUT, "TEXT2");
    ti3.init(TEXT3_INPUT, "TEXT3");
  }

  void RedrawText() {
    if (main_text_framebuffer != nullptr) {
      main_text_framebuffer->setDirty();
    }
  }

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "width", json_integer(width));
    json_object_set_new(rootJ, "timestamp", json_integer(time_stamp_format));

    json_object_set_new(rootJ, "screen_colors", json_integer(screen_colors));
    if (font_choice.length() > 0) {
      json_object_set_new(rootJ, "font_choice",
                          json_stringn(font_choice.c_str(), font_choice.size()));
    }
    json_object_set_new(rootJ, "visible_lines", json_integer(visible_lines));
    if (preface_outputs) {
      json_object_set_new(rootJ, "preface_outputs", json_integer(1));
    }
    if (preserve_output) {
      json_object_set_new(rootJ, "preserve_output", json_integer(1));
      json_object_set_new(rootJ, "text", json_stringn(text.c_str(), text.size()));
    }
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* widthJ = json_object_get(rootJ, "width");
    if (widthJ)
      width = json_integer_value(widthJ);
    json_t* timestampJ = json_object_get(rootJ, "timestamp");
    if (timestampJ)
      time_stamp_format = static_cast<TimeStamp>(json_integer_value(timestampJ));
    json_t* screenJ = json_object_get(rootJ, "screen_colors");
    if (screenJ)
        screen_colors = json_integer_value(screenJ);
    json_t* font_choiceJ = json_object_get(rootJ, "font_choice");
    if (font_choiceJ) {
      font_choice = json_string_value(font_choiceJ);
    }
    json_t* visible_linesJ = json_object_get(rootJ, "visible_lines");
    if (visible_linesJ)
        visible_lines = json_integer_value(visible_linesJ);
    json_t* preface_outputsJ = json_object_get(rootJ, "preface_outputs");
    if (preface_outputsJ) {
      preface_outputs = json_integer_value(preface_outputsJ) == 1;
    } else {
      preface_outputs = false;
    }
    json_t* preserve_outputJ = json_object_get(rootJ, "preserve_output");
    if (preserve_outputJ) {
      preserve_output = json_integer_value(preserve_outputJ) == 1;
      if (preserve_output) {
        json_t* textJ = json_object_get(rootJ, "text");
        if (textJ) {
          text = json_string_value(textJ);
          cursor_override = 2000000;
          editor_refresh = true;
        }
      }
    } else {
      preserve_output = false;
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

  void processTextInput(bool paused, TextInput *text_input) {
    if (inputs[text_input->id].isConnected()) {
      auto decoder_status = text_input->decoder.readFloat(
          inputs[text_input->id].getVoltage());
      if (!paused && !text_input->decoder.isError(decoder_status) &&
          decoder_status == tipsy::DecoderResult::BODY_READY &&
          std::strcmp(text_input->decoder.getMimeType(), "text/plain") == 0) {
        std::string str_value(std::string((const char *) text_input->recvBuffer));
        if (str_value.compare("!!CLEAR!!") == 0) {
          clear_command_received = true;
        } else {
          std::string prefix(MakePrefix(text_input->preface));
          std::string next(prefix);
          next.append(str_value);
          next.append("\n");
          add_string(next);
        }
      }
    }
  }

  std::string MakePrefix(const std::string& source) {
    std::string result;
    switch (time_stamp_format) {
      case NONE:
        break;
      case PATCH_TIME: {
        double seconds = rack::system::getTime();
        int hours = static_cast<int>(seconds / 3600);
        seconds -= hours * 3600;
        int minutes = static_cast<int>(seconds / 60);
        seconds -= minutes * 60;
        int secs = static_cast<int>(seconds);

        char buffer[9]; // Enough space for HH:MM:SS
        sprintf(buffer, "%02d:%02d:%02d", hours, minutes, secs);
        result.append(buffer);
      }
      break;
      case LOCAL_TIME: {
        time_t t = system::getUnixTime();
        char buffer[9];
        size_t s = std::strftime(buffer, sizeof(buffer), "%H:%M:%S", std::localtime(&t));
        result.append(std::string(buffer, s));
      }
      break;
    }

    if (preface_outputs) {
      if (!result.empty()) {
        result.append(" ");
      }
      result.append(source);
      result.append(">");
    }
    if (!result.empty()) {
      result.append(" ");
    }
    return result;
  }

  bool float_equal(float f1, float f2) {
    static constexpr auto epsilon = 1.0e-05f;
    if (std::fabs(f1 - f2) <= epsilon)
        return true;
    return std::fabs(f1 - f2) <= epsilon * fmax(std::fabs(f1), std::fabs(f2));
  }
 
  void process(const ProcessArgs& args) override {
    bool paused = params[PAUSE_PARAM].getValue() > 0;

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
      if (tick_count > params[SAMPLE_PARAM].getValue() * args.sampleRate / 1000) {
        tick_count = 0;
        if (inputs[V1_INPUT].isConnected()) {
          float v1 = inputs[V1_INPUT].getVoltage();
          if (!float_equal(v1, previous_v1)) {
            previous_v1 = v1;
            std::string str_value = std::to_string(v1);
            // Hmmmm; should I be comparing the string values instead? It would be
            // odd to see the same string twice on a tightly changing value....

            // Add to buffer.
            std::string prefix(MakePrefix("V1"));
            std::string next(prefix);
            next.append(str_value);
            next.append("\n");
            add_string(next);
          }
        }
        if (inputs[V2_INPUT].isConnected()) {
          float v2 = inputs[V2_INPUT].getVoltage();
          if (!float_equal(v2, previous_v2)) {
            previous_v2 = v2;
            std::string str_value = std::to_string(v2);
            // Hmmmm; should I be comparing the string values instead? It would be
            // odd to see the same string twice on a tightly changing value....

            // Add to buffer.
            std::string prefix(MakePrefix("V2"));
            std::string next(prefix);
            next.append(str_value);
            next.append("\n");
            add_string(next);
          }
        }
      }
    }

    // Processing text can set this to true.
    clear_command_received = false;
    processTextInput(paused, &ti1);
    processTextInput(paused, &ti2);
    processTextInput(paused, &ti3);
    if (clear_command_received) {
      clear_light_countdown = std::floor(args.sampleRate / 10.0f);
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
      RedrawText();
    }

    // Lights.
    if (clear_light_countdown > 0) {
      clear_light_countdown--;
    }
    lights[CLEAR_LIGHT].setBrightness(
      clear_light_countdown > 0 || clear ? 1.0f : 0.0f);
    lights[PAUSE_LIGHT].setBrightness(paused);
  }

  TextInput ti1, ti2, ti3;

  bool clear_command_received;
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
  // Preface outputs with source port when true; set via context menu.
  bool preface_outputs = false;
  // Save/restore output when saving/loading the patch.
  bool preserve_output = false;
  // What timestamp to use, if any.
  TimeStamp time_stamp_format = NONE;

  // Black on yellow paper color (by my memory, at least).
  long long int screen_colors = 0x000000edc672;
  int visible_lines = 28;
  // width (in "holes") of the whole module. Changed by the resize bar on the
  // right (within limits), and informs the size of the display and text field.
  // Saved in the json for the module.
  int width = TTY::DEFAULT_WIDTH;
  // The undo/redo sometimes needs to reset the cursor position.
  // But we don't actully have a good pointer to the text field.
  // drawLayer() uses this if it's > -1.
  int cursor_override = -1;
  // Can be overriden by saved menu choice.
  std::string font_choice = "fonts/RobotoMono-Regular.ttf";
  // Many actions mean we need to force the buffer to recalculate the
  // text appearance, so we keep the FramebufferWidget available.
  FramebufferWidget* main_text_framebuffer = nullptr;
};

// Adds support for undo/redo in the text field where people type programs.
struct TTYUndoRedoAction : history::ModuleAction {
  int old_width, new_width;

  TTYUndoRedoAction(int64_t id, int old_width, int new_width) :
      old_width{old_width}, new_width{new_width} {
    moduleId = id;
    name = "module width change";
  }
  void undo() override {
    TTY *module = dynamic_cast<TTY*>(APP->engine->getModule(moduleId));
    if (module) {
      module->width = this->old_width;
      module->RedrawText();
    }
  }

  void redo() override {
    TTY *module = dynamic_cast<TTY*>(APP->engine->getModule(moduleId));
    if (module) {
      module->width = this->new_width;
      module->RedrawText();
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
    const float maxWidth = 300 * RACK_GRID_WIDTH;
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
      // Make resizing an undo/redo action. If I don't do this, undoing a
      // different module's move will cause them to overlap (aka, a
      // transporter malfunction).
      APP->history->push(
        new TTYUndoRedoAction(module->id, original_width, module->width));
      // Also need to tell FramebufferWidget to update the appearance,
      // since the width has changed.
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

static std::string module_browser_text =
  "Logs DISTINCT values coming in through V1 or V2.\n"
  "Logs Tipsy text messages sent by BASICally or Memory.\n"
  "Horizontally resizable.";

// Class for the editor.
struct TTYTextField : STTextField {
  TTY* module;
  FramebufferWidget* frame_buffer;
  bool was_selected;
  long long int color_scheme;
  std::unordered_map<int, std::pair<int, int>> lines_to_font_size_and_offset;

  TTYTextField() {
    for (int index = 0; index < 13; index++) {
      lines_to_font_size_and_offset.insert({LARGER_TEXT_INFO[index][0],
         std::make_pair(LARGER_TEXT_INFO[index][1], LARGER_TEXT_INFO[index][2])});
    }
  }

  void set_visible_lines(int visible_lines) {
    std::unordered_map<int, std::pair<int, int>>::const_iterator found =
       lines_to_font_size_and_offset.find(visible_lines);
    if (found == lines_to_font_size_and_offset.end()) {
      fontSize = 12;
      textOffset = math::Vec(3, 3);
    } else {
      fontSize = found->second.first;
      textOffset = math::Vec(3, (float) (found->second.second));
    }
    // TTY prefers to put the cursor on the line after the last text,
    // which makes much of the text window blank at large font sizes.
    // TODO: maybe this has to do with the 2nd arg to extended.Initialize() below?
    large_text_mode = visible_lines <= 6 ;
    // At fontsize 12, it's 28 rows.
    // When only a couple of lines high, scrolling before getting to top or bottom
    // is frustrating. 
    extended.Initialize(visible_lines, visible_lines >= 6 ? 1 : 0);
    textUpdated();
  }

  NVGcolor int_to_color(int color) {
    return nvgRGB(color >> 16, (color & 0xff00) >> 8, color & 0xff);
  }

  // Setting the font.
  void setFontPath() {
    if (module) {
      fontPath = module->getFontPath();
    }
  }

  void setModule(TTY* module, FramebufferWidget* fb_widget) {
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
  // and then letting LedDisplayTextField draw the rest will fixes that.
  void draw(const DrawArgs& args) override {
    nvgScissor(args.vg, RECT_ARGS(args.clipBox));

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
    STTextField::draw(args);  // Draw text.
    nvgResetScissor(args.vg);
  }

  void step() override {
    STTextField::step();
    if (module) {
      // At smaller sizes, hide the screen.
      if (module->width <= 4) {
        frame_buffer->hide();
      } else {
        frame_buffer->show();
      }
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
      if (module && (fabs(fontSize - module->visible_lines) > 0.1)) {
        set_visible_lines(module->visible_lines);
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
    // Need to notice when the text window has become (or no longer is)
    // the focus, since that determines if we show the cursor or not.
    bool is_selected = (this == APP->event->selectedWidget);
    if (is_selected != was_selected) {
      was_selected = is_selected;
      is_dirty = true;
    }   
    // If ANYTHING thinks we should redraw, this makes it happen.
    if (is_dirty && frame_buffer) {
      frame_buffer->setDirty();
    }
  }
};

const float NON_SCREEN_WIDTH = 2.0f;
const float CONTROL_WIDTH = 13.0f;

struct TTYWidget : ModuleWidget {
  Widget* topRightScrew;
  Widget* bottomRightScrew;
  TTYModuleResizeHandle* rightHandle;
  TTYTextField* textDisplay;
  FramebufferWidget* main_text_framebuffer;

  TTYWidget(TTY* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/TTY.svg"),
                         asset::plugin(pluginInstance, "res/TTY-dark.svg")));

    // Set reasonable initial size of module. Will likely get updated below.
    box.size = Vec(RACK_GRID_WIDTH * TTY::DEFAULT_WIDTH, RACK_GRID_HEIGHT);
    if (module) {
      // Set box width from loaded Module when available.
      box.size.x = module->width * RACK_GRID_WIDTH;
    } else {
      // Like when showing the module in the module browser.
      box.size.x = TTY::DEFAULT_WIDTH * RACK_GRID_WIDTH;
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

    addParam(createParamCentered<RoundBlackKnob>(
         mm2px(Vec(8.938, 22.329)), module, TTY::SAMPLE_PARAM));
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(8.938, 59.0)),
                                             module, TTY::PAUSE_PARAM,
                                             TTY::PAUSE_LIGHT));
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(8.938, 71.0)),
                                             module, TTY::CLEAR_PARAM,
                                             TTY::CLEAR_LIGHT));

    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.938, 34.663)), module,
        TTY::V1_INPUT));
    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.938, 45.546)), module,
        TTY::V2_INPUT));
    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.938, 86.0)), module,
        TTY::TEXT1_INPUT));
    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.938, 102.0)), module,
        TTY::TEXT2_INPUT));
    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.938, 118.0)), module,
        TTY::TEXT3_INPUT));

    // The FramebufferWidget that caches the appearence of the text, so we
    // don't have to keep redrawing it (and wasting UI CPU to do it).
    main_text_framebuffer = new FramebufferWidget();
    textDisplay = createWidget<TTYTextField>(mm2px(Vec(18.08, 5.9)));
    textDisplay->box.size = mm2px(Vec(60.0, 117.0));
    textDisplay->box.size.x = box.size.x - RACK_GRID_WIDTH * NON_SCREEN_WIDTH - mm2px(CONTROL_WIDTH);
    textDisplay->setModule(module, main_text_framebuffer);
    addChild(main_text_framebuffer);
    main_text_framebuffer->addChild(textDisplay);
    if (module) {
      module->main_text_framebuffer = main_text_framebuffer;
    }

    // Resize bar on right.
    rightHandle = new TTYModuleResizeHandle;
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
    menu->addChild(new MenuSeparator);

    // Some functionality choices.
    menu->addChild(createBoolMenuItem("Preface lines with source port", "",
                                      [=]() {return module->preface_outputs;},
                                      [=](bool state) {module->preface_outputs = state;
                                                       module->RedrawText();}
                                      ));
    // Select timestamp to show.
    std::pair<std::string, TTY::TimeStamp> time_options[] = {
      {"No", TTY::NONE},
      {"Time since patch started", TTY::PATCH_TIME},
      {"Local time", TTY::LOCAL_TIME}
    };
    MenuItem* time_menu = createSubmenuItem("Preface lines with timestamp?", "",
     [=](Menu* menu) {
         for (auto line : time_options) {
           menu->addChild(createCheckMenuItem(line.first, "",
           [=]() {return line.second == module->time_stamp_format;},
           [=]() {module->time_stamp_format = line.second;}
           ));
         }
     }
    );
    menu->addChild(time_menu);

    menu->addChild(createBoolPtrMenuItem("Keep recent output when patch is saved", "",
                                          &module->preserve_output));

    // Add color choices.
    menu->addChild(new MenuSeparator);
    std::pair<std::string, long long int> colors[] = {
      {"Green on Black", 0x00ff00000000},
      {"White on Black", 0xffffff000000},
      {"Yellow on Black (like Notes)", 0xffd714000000},
      {"Amber on Black", 0xffc000000000},
      {"Blue on Black", 0x29b2ef000000},
      {"Black on White", 0x000000ffffff},
      {"Black on Yellow (TTY Paper)", 0x000000edc672},
      {"Blue on White", 0x29b2efffffff}
    };
    MenuItem* color_menu = createSubmenuItem("Screen Colors", "",
     [=](Menu* menu) {
         for (auto line : colors) {
           menu->addChild(createCheckMenuItem(line.first, "",
           [=]() {return line.second == module->screen_colors;},
           [=]() {module->screen_colors = line.second;
                  module->RedrawText();}
           ));
         }
     }
    );
    menu->addChild(color_menu);

    MenuItem* visible_lines_menu = createSubmenuItem("Visible Lines", "",
      [=](Menu* menu) {
          for (int index = 0; index < 13; index++) {
            int lines = LARGER_TEXT_INFO[index][0];
            menu->addChild(createCheckMenuItem(std::to_string(lines), "",
                [=]() {return lines == module->visible_lines;},
                [=]() {module->visible_lines = lines;
                       textDisplay->set_visible_lines(module->visible_lines);
                       module->RedrawText();
                      }
            ));
          }
      }
    );
    menu->addChild(visible_lines_menu);

    std::pair<std::string, std::string> fonts[] = {
      {"VCV font (like Notes)", "res/fonts/ShareTechMono-Regular.ttf"},
      {"Veteran Typewriter (TTY-ish font)", "fonts/veteran-typewriter.regular.ttf"},
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
                       textDisplay->setFontPath();
                       module->RedrawText();}
            ));
          }
      }
    );
    menu->addChild(font_menu);

  }
};

Model* modelTTY = createModel<TTY, TTYWidget>("TTY");
