#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <vector>

#include "extended_text.h"
#include "plugin.hpp"
#include "parser/driver.hh"
#include "pcode.h"
#include "st_textfield.hpp"

enum Style {
  ALWAYS_STYLE,
  TRIGGER_LOOP_STYLE,
  TRIGGER_NO_LOOP_STYLE,
  GATE
};

Style STYLES[] = {
  ALWAYS_STYLE,
  TRIGGER_LOOP_STYLE,
  TRIGGER_NO_LOOP_STYLE,
  GATE
};

// The execution of WAIT statements needs to be as efficient as I can make them.
// This data structure just collects all data about the ongoing WAITs.
struct WaitInfo {
  int countdown_to_recompute;
  // Are we currently in a WAIT?
  bool in_wait;
  // The expression that computes the wait time could change while the wait is
  // occuring.
  bool is_volatile;
  // The inputs the WAIT Expression relies on, if any.
  std::unordered_set<std::string> volatile_deps;
  // The number of process() calls seen in this WAIT. GOes up by one every time
  // procees is called during the wait.
  int ticks_so_far;
  // The number of ticks needed to complete the wait. If is_volatile, then
  // this needs to be recomputed when INn gets changed.
  int ticks_limit;
};

struct Basically : Module {
  enum ParamId {
    RUN_PARAM,
    STYLE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    IN1_INPUT,
    IN2_INPUT,
    IN3_INPUT,
    IN4_INPUT,
    RUN_INPUT,
    IN5_INPUT,
    IN6_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    OUT1_OUTPUT,
    OUT2_OUTPUT,
    OUT3_OUTPUT,
    OUT4_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    RUN_LIGHT,   // Currently running.
    LIGHTS_LEN
  };

  // Structure that maps text name to enum to float* for I/O variables.
  struct InPortInfo {
    std::string name;
    InputId id;
    float* value_ptr;

    InPortInfo(const std::string a, InputId b) : name{a}, id{b} {
      value_ptr = nullptr;
    }
  };

  std::unordered_map<std::string, OutputId> out_map { {"out1", OUT1_OUTPUT},
                                                      {"out2", OUT2_OUTPUT},
                                                      {"out3", OUT3_OUTPUT},
                                                      {"out4", OUT4_OUTPUT}
                                                    };
  std::vector<InPortInfo> in_list { {"in1", IN1_INPUT},
                                    {"in2", IN2_INPUT},
                                    {"in3", IN3_INPUT},
                                    {"in4", IN4_INPUT},
                                    {"in5", IN5_INPUT},
                                    {"in6", IN6_INPUT}
                                  };
  // width (in "holes") of the whole module. Changed by the resize bar on the
  // right (within limits), and informs the size of the display and text field.
  // Saved in the json for the module.
  int width = 16;

  Basically() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configButton(RUN_PARAM, "Press to run");
    configSwitch(STYLE_PARAM, 0, 3, 0, "When to run",
                 {"Always run", "Start on trigger, loop",
                  "Start on trigger, don't loop",
                  "Run when gate is open"});
    // This has distinct values.
    getParamQuantity(STYLE_PARAM)->snapEnabled = true;
    configInput(IN1_INPUT, "IN1");
    configInput(IN2_INPUT, "IN2");
    configInput(IN3_INPUT, "IN3");
    configInput(IN4_INPUT, "IN4");
    configInput(IN5_INPUT, "IN5");
    configInput(IN6_INPUT, "IN6");
    configInput(RUN_INPUT, "Trigger to start or Gate to start/stop (See Style)");
    configOutput(OUT1_OUTPUT, "OUT1");
    configOutput(OUT2_OUTPUT, "OUT2");
    configOutput(OUT3_OUTPUT, "OUT3");
    configOutput(OUT4_OUTPUT, "OUT4");
    configLight(RUN_LIGHT, "Lit when code is currently running.");

    current_line = 0;
    wait_info.in_wait = false;

    // Add the INn variables to the variable space, and get the pointer to
    // them so module can set them.
    for (size_t i = 0; i < in_list.size(); i++) {
      in_list[i].value_ptr = drv.GetVarFromName(in_list[i].name);
    }
    // Add the OUTn variables to the symbol table, just so we're sure they
    // are present. But we don't appear to need to keep the pointers in
    // the module.
    for (auto output : out_map) {
      drv.GetVarFromName(output.first);
    }
  }

  // If asked to, save the curve data in json for reading when loaded.
  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "text", json_stringn(text.c_str(), text.size()));
    json_object_set_new(rootJ, "width", json_integer(width));
    if (allow_error_highlight) {
      json_object_set_new(rootJ, "allow_error_highlight", json_integer(1));
    }
    json_object_set_new(rootJ, "screen_colors", json_integer(screen_colors));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* textJ = json_object_get(rootJ, "text");
		if (textJ) {
			text = json_string_value(textJ);
  		dirty = true;
    }
    json_t* widthJ = json_object_get(rootJ, "width");
		if (widthJ)
			width = json_integer_value(widthJ);
    json_t* screenJ = json_object_get(rootJ, "screen_colors");
		if (screenJ)
			screen_colors = json_integer_value(screenJ);
    json_t* error_highlightJ = json_object_get(rootJ, "allow_error_highlight");
    if (error_highlightJ) {
      allow_error_highlight = json_integer_value(error_highlightJ) == 1;
    } else {
      allow_error_highlight = false;
    }
  }

  void ResetToProgramStart() {
    current_line = 0;
    wait_info.in_wait = false;
    // Do we need a gesture that clears all variables? Likely not often;
    // keeping previously defined variables makes live-coding work.
  }

  void UpdateOutsIfNeeded(const std::string var_name, float value) {
    auto found = out_map.find(var_name);
    if (found != out_map.end()) {
      // Limit to -10 <= x < = 10.
      outputs[found->second].setVoltage(
        std::max(-10.0f, std::min(10.0f, value)));
    }
  }

  // Update all connected inputs.
  void UpdateAllInputs() {
    for (auto input : in_list) {
      // Unconnected inputs can't change.
      if (inputs[input.id].isConnected()) {  // TODO: is this an expensive call?
        *(input.value_ptr) = inputs[input.id].getVoltage();
      }
    }
  }

  // Version of UpdateAllInputs optimized for volatile WAITs.
  // Update only the inputs listed in deps.
  // Returns true iff any input listed in deps changed value.
  bool UpdateNeededInputs(const std::unordered_set<std::string> &deps) {
    bool dep_changed = false;
    for (auto input : in_list) {
      // Unconnected inputs can't change.
      if (inputs[input.id].isConnected()) {
        if (deps.find(input.name) != deps.end()) {
          // TODO: _maybe_ slightly faster to pull the pair from the map?
          float prev = *(input.value_ptr);
          float new_value = inputs[input.id].getVoltage();
          if (!Expression::is_zero(new_value - prev)) {
            *(input.value_ptr) = new_value;
            dep_changed = true;
          }
        }
      }
    }
    return dep_changed;
  }

  void processBypass(const ProcessArgs& args) override {
    outputs[OUT1_OUTPUT].setVoltage(0.0f);
    outputs[OUT2_OUTPUT].setVoltage(0.0f);
    outputs[OUT3_OUTPUT].setVoltage(0.0f);
    outputs[OUT4_OUTPUT].setVoltage(0.0f);
  }

  void process(const ProcessArgs& args) override {
    Style style = STYLES[(int) params[STYLE_PARAM].getValue()];
    bool loops = (style != TRIGGER_NO_LOOP_STYLE);

    // Compile if we need to.
    if (user_has_changed && !text.empty()) {
      user_has_changed = false;  // TODO: race condition?
      std::string lowercase;
      lowercase.resize(text.size());
      std::transform(text.begin(), text.end(),
                     lowercase.begin(), ::tolower);
      compiles = !drv.parse(lowercase);
      if (compiles) {
        PCodeTranslator translator;
        translator.LinesToPCode(drv.lines, &pcodes);
        /*
        for (auto &pcode : pcodes) {
          // Add to log, for debugging.
          INFO("%s", pcode.to_string().c_str());
        }
        */
        // Recompiled; cannot trust program state.
        ResetToProgramStart();
      }
    }

    // Determine if we are running or not.
    // This section is more complicated than needed to avoid unneeded
    // work every process() call.
    bool prev_running = running;
    if (style == ALWAYS_STYLE) {
      if (compiles) running = true;
    } else if (style == TRIGGER_LOOP_STYLE || style == TRIGGER_NO_LOOP_STYLE) {
      if (!running) {
        bool run_was_low = !runTrigger.isHigh();
        runTrigger.process(rescale(
            inputs[RUN_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
        bool run_button_pressed = params[RUN_PARAM].getValue() > 0.1f;
        bool asked_to_start_run = (run_button_pressed ||
                                   (run_was_low && runTrigger.isHigh()));
        if (asked_to_start_run) {
          running = true;
          ResetToProgramStart();
        }
      }
    } else {  // style == GATE
      runTrigger.process(rescale(
          inputs[RUN_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
      if (runTrigger.isHigh()) {
        running = true;
      } else {
        running = params[RUN_PARAM].getValue() > 0.1f;
      }
    }
    if (pcodes.size() == 0) {
      // No lines to run mean don't run!
      running = false;
    }
    if (!prev_running && running) {
      // Flash the run light for a brief part of  asecond.
      // Compute how many samples to show the light.
      run_light_countdown = std::floor(args.sampleRate / 20.0f);
    }

    // Update INn variables with current inputs, but only if we _need_ to.
    bool need_to_update_wait = false;
    // Need to determine if:
    // * We need to update the inputs at all.
    // * We are in a WAIT *and* we need to recompute the wait time.
    // These are related questions.
    if (running) {
      // If we're not running, don't need to update inputs, and the WAIT
      // status doesn't change.
      if (!wait_info.in_wait) {
        // Not in a WAIT => Definitely need to update.
        UpdateAllInputs();
      } else {
        // In a WAIT, but does the wait interval rely on inputs?
        if (wait_info.is_volatile) {
          wait_info.countdown_to_recompute--;
          if (wait_info.countdown_to_recompute <= 0) {
            // Looks like inputs contribute to the WAIT period we are currently
            // in. Better update the ones I need.
            need_to_update_wait = UpdateNeededInputs(wait_info.volatile_deps);
            wait_info.countdown_to_recompute = std::floor(
                args.sampleRate / 1000.0f);
          }
        }
      }
    }

    // Run the PCode vector from the current spot in it.
    bool waiting = false;

    while (running && !waiting) {
      PCode* pcode = &(pcodes[current_line]);
      switch (pcode->type) {
        case PCode::ASSIGNMENT: {
          float rhs = pcode->expr1.Compute();
          *(pcode->variable_ptr) = rhs;
          UpdateOutsIfNeeded(pcode->str1, rhs);
          current_line++;
        }
        break;
        case PCode::WAIT: {
          if (wait_info.in_wait) {
            // We're currently running through this statement's wait period.
            // Update it by one tick.
            wait_info.ticks_so_far++;
            // If the wait period may have changed, recompute it.
            if (need_to_update_wait) {
              wait_info.ticks_limit = std::floor(
                  pcode->expr1.Compute() * args.sampleRate / 1000.0f);
            }
            if (wait_info.ticks_so_far >= wait_info.ticks_limit) {
              // WAIT has completed, immediately execute next line.
              wait_info.in_wait = false;
              current_line++;
              // We may not updated *all* of the Inputs, let's do so.
              UpdateAllInputs();
            }
          } else {
            // Just arriving at this WAIT statement.
            int ticks = std::floor(
                pcode->expr1.Compute() * args.sampleRate / 1000.0f);
            // A "WAIT 0" (or WAIT -1!) means we should stop running for
            // this process() call but push to the next line. No reason to
            // create a WaitInfo.
            if (ticks <= 0) {
              current_line++;
              waiting = true;
            } else {
              // This WAIT is longer than a single tick, so we should fill in
              // the WaitInfo for it.
              wait_info.is_volatile = pcode->expr1.Volatile(
                &(wait_info.volatile_deps));
              // It wastes a lot of CPU seeing if we need to recompute every tick.
              // We instead only consider doing so every millisecond.
              wait_info.countdown_to_recompute =
                 std::floor(args.sampleRate / 1000.0f);
              wait_info.ticks_limit = ticks;
              wait_info.ticks_so_far = 0;
              wait_info.in_wait = true;
            }
          }
          if (wait_info.in_wait) {
            waiting = true;
          }
        }
        break;
        case PCode::IFNOT: {
          // All this PCode does is determine where to move current_line to.
          bool expr_val = !Expression::is_zero(pcode->expr1.Compute());
          if (!expr_val) {
            current_line += pcode->jump_count;
          } else {
            current_line++;
          }
        }
        break;
        case PCode::RELATIVE_JUMP: {
          if (pcode->stop_execution) {
            ResetToProgramStart();
            running = false;  // TODO: is the the correct behavior for all Styles?
          } else {
            // This just specifies a jump of the current_line.
            current_line += pcode->jump_count;
          }
        }
        break;
        case PCode::FORLOOP: {
          if (state == PCode::ENTERING_FOR_LOOP) {
            pcode->limit = pcode->expr1.Compute();
            pcode->step = pcode->expr2.Compute();
          } else {
            float new_value = *(pcode->variable_ptr) + pcode->step;
            *(pcode->variable_ptr) = new_value;
            UpdateOutsIfNeeded(pcode->str1, new_value);
          }
          bool done = false;
          // If "Step" is negative, we wait until value is below limit.
          if (pcode->step >= 0.0f) {
            done = *(pcode->variable_ptr) > pcode->limit;
          } else {
            done = *(pcode->variable_ptr) < pcode->limit;
          }
          if (done) {
            current_line += pcode->jump_count;
          } else {
            current_line++;
          }
        }
      }
      state = pcode->state;
      if (current_line >= pcodes.size()) {
        current_line = 0;
        waiting = true;  // Implicit WAIT at end of program.
        if (!loops) {
          running = false;
        }
      }
    }

    // Lights.
    if (run_light_countdown > 0) {
      run_light_countdown--;
    }
    lights[RUN_LIGHT].setBrightness(
      (running || run_light_countdown > 0) ? 1.0f : 0.0f);
  }

  dsp::SchmittTrigger runTrigger;
  std::string text;
  bool dirty = false;  // Set when module changes the text (like at start).
  bool user_has_changed = true;
  bool compiles = false;
  bool running = false;
  bool allow_error_highlight = true;
  Driver drv;
  std::vector<PCode> pcodes;  // What actually gets executed.
  // Line where execution is currently happening.
  unsigned int current_line;
  // Some PCodes have different behaviors, depending on how execution got
  // there. 'state' helps determine the correct behavior.
  PCode::State state;
  // Green on Black.
  long long int screen_colors = 0x00ff00000000;
  // Keeps lights lit long enough to see.
  int run_light_countdown = 0;
  WaitInfo wait_info;
};

// Adds support for undo/redo in the text field where people type programs.
struct TextEditAction : history::ModuleAction {
  std::string old_text;
  std::string new_text;

  TextEditAction(int64_t id, std::string oldText, std::string newText) {
    moduleId = id;
    name = "edit code";
    old_text = oldText;
    new_text = newText;
  }
  void undo() override {
    Basically *module = dynamic_cast<Basically*>(APP->engine->getModule(moduleId));
    if (module) {
      module->text = this->old_text;
      module->dirty = true;  // Tell UI it needs to reload 'text'.
      module->user_has_changed = true;  // Tell compiler it needs to re-evaluate 'text'.
    }
  }

  void redo() override {
    Basically *module = dynamic_cast<Basically*>(APP->engine->getModule(moduleId));
    if (module) {
      module->text = this->new_text;
      module->dirty = true;  // Tell UI it needs to reload 'text'.
      module->user_has_changed = true;  // Tell compiler it needs to re-evaluate 'text'.
    }
  }
};

struct ModuleResizeHandle : OpaqueWidget {
	Vec dragPos;
	Rect originalBox;
	Basically* module;

	ModuleResizeHandle() {
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

		// Set box and test whether it's valid
		mw->box = newBox;
		if (!APP->scene->rack->requestModulePos(mw, newBox.pos)) {
			mw->box = oldBox;
		}
		module->width = std::round(mw->box.size.x / RACK_GRID_WIDTH);
	}

	void draw(const DrawArgs& args) override {
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
};

struct BasicallyTextField : STTextField {
	Basically* module;
  ExtendedText extended;  // Helper for navigating a long string.
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

      // Let's try highlighting a line, though, if needed and allowed to.
      if (module && module->allow_error_highlight) {
        // Highlight the line with an error, if any.
        if (module->drv.errors.size() > 0) {
          int line_number = module->drv.errors[0].line;
          nvgBeginPath(args.vg);
          int topFudge = textOffset.y + 5;  // I'm just trying things until they work.
          // textOffset is in ledDisplayTextField.
          nvgRect(args.vg, 0, topFudge + 12 * (line_number - 1), box.size.x, 12);
          nvgFillColor(args.vg, nvgRGB(128, 0, 0));
          nvgFill(args.vg);
        }
      }
  	}
    // If this comes out wrong, pull in the LedDisplayTextField version.
  	STTextField::drawLayer(args, layer);  // Draw text.
  	nvgResetScissor(args.vg);
  }

	void step() override {
		STTextField::step();
    if (module && (color_scheme != module->screen_colors || module->dirty)) {
      color_scheme = module->screen_colors;
      color = int_to_color(color_scheme >> 24);
      // LedDisplay, which is doing the actual drawing, seems to ignore
      // bgColor. Keeping this for if/when we replace LedDisplay.
      bgColor = int_to_color(color_scheme & 0xffffff);
    }
		if (module && module->dirty) {
      // Text has been changed by the module (not the user).
      // This happens when the module loads.
      // Index the lines by calling this.
      extended.ProcessUpdatedText(module->text);
			setText(module->text);
			module->dirty = false;
		}
	}

  // So we can handle up and down keys.
  void onSelectKey(const SelectKeyEvent& e) override {
    if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
      // Up (placeholder)
  		if (e.key == GLFW_KEY_UP) {
        // Move to same column, in previous line.
        LineColumn lc = extended.GetCurrentLineColumn(cursor);
        cursor = extended.GetCursorForLineColumn(lc.line - 1, lc.column);
        if (!(e.mods & GLFW_MOD_SHIFT)) {
  				selection = cursor;  // Otherwise we select the line.
  			}
  			e.consume(this);
  		}
  		// Down (placeholder)
  		if (e.key == GLFW_KEY_DOWN) {
        // Move to same column, in next line.
        LineColumn lc = extended.GetCurrentLineColumn(cursor);
        cursor = extended.GetCursorForLineColumn(lc.line + 1, lc.column);
        if (!(e.mods & GLFW_MOD_SHIFT)) {
  				selection = cursor;  // Otherwise we select the line.
  			}
  			e.consume(this);
  		}
    }
    STTextField::onSelectKey(e);
  }

  // User has updated the text.
	void onChange(const ChangeEvent& e) override {
		if (module) {
      // Create a ModuleAction so this can undo/redo is aware of it.
      std::string new_text = getText();
      // Sometimes the text isn't actually different. If I don't check
      // this, I get spurious history events.
      if (module->text != new_text) {
        APP->history->push(
          new TextEditAction(module->id, module->text, new_text));
        extended.ProcessUpdatedText(new_text);
        module->text = new_text;
        module->user_has_changed = true;
      }
    }
	}
};

struct BasicallyDisplay : LedDisplay {
  BasicallyTextField* textField;

	void setModule(Basically* module) {
		textField = createWidget<BasicallyTextField>(Vec(0, 0));
		textField->box.size = box.size;
		textField->multiline = true;
		textField->module = module;
		addChild(textField);
	}
  // The BasicallyWidget changes size, so we have to reflaect that.
  void step() override {
    textField->box.size = box.size;
    LedDisplay::step();
	}

  // Text insertions from the menu.
  void insertText(const std::string &fragment) {
    textField->insertText(fragment);
  }
};

struct ErrorWidget;
struct ErrorTooltip : ui::Tooltip {
	ErrorWidget* errorWidget;
  std::string error_text;

  ErrorTooltip(const std::string &text) : error_text{text} {}

	void step() override;
};

struct ErrorWidget : widget::OpaqueWidget {
  Basically* module;
  std::string fontPath;
  ErrorTooltip* tooltip;

  ErrorWidget() {
    fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
    tooltip = NULL;
  }

  void onEnter(const EnterEvent & e) override {
    create_tooltip();
  }

  void onLeave(const LeaveEvent & e) override {
    destroy_tooltip();
  }

  void create_tooltip() {
  	if (!settings::tooltips)
  		return;
  	if (tooltip)  // Already exists.
  		return;
  	if (!module)
  		return;
    std::string tip_text;
    if (module->compiles) {
      tip_text = "Program compiles!";
    } else {
      if (module->text.empty()) {
        tip_text = "Type in some code over there ->";
      } else {
        if (module->drv.errors.size() > 0) {
          Error err = module->drv.errors[0];
          // Remove "syntax error, " from message.
          std::string msg = err.message;
          if (msg.rfind("syntax error, ", 0) == 0) {
            msg = msg.substr(14);
          }
          tip_text = "Line " + std::to_string(err.line) + ": " + msg;
        }
      }
    }
  	ErrorTooltip* new_tooltip = new ErrorTooltip(tip_text);
  	new_tooltip->errorWidget = this;
  	APP->scene->addChild(new_tooltip);
  	tooltip = new_tooltip;
  }

  void destroy_tooltip() {
  	if (!tooltip)
  		return;
  	APP->scene->removeChild(tooltip);
  	delete tooltip;
  	tooltip = NULL;
  }

  void drawLayer(const DrawArgs& args, int layer) override {
    if ((layer == 1) && module) {
      Rect r = box.zeroPos();
      Vec bounding_box = r.getBottomRight();
      // Fill the rectangle with either Green or Red.
      NVGcolor main_color = (module->compiles ? SCHEME_GREEN : color::RED);
      nvgBeginPath(args.vg);
      nvgRect(args.vg, 0.5, 0.5,
              bounding_box.x - 1.0f, bounding_box.y - 1.0f);
      nvgFillColor(args.vg, main_color);
      nvgFill(args.vg);

      std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
      if (font) {
        // WHITE is really hard to read on YELLOW.
        nvgFillColor(args.vg, color::BLACK);
        nvgFontSize(args.vg, 13);
        nvgTextAlign(args.vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
        nvgFontFaceId(args.vg, font->handle);
        nvgTextLetterSpacing(args.vg, -2);

        std::string text = (module->compiles ? "Good" : "Fix");
        // Place on the line just off the left edge.
        nvgText(args.vg, bounding_box.x / 2, 0, text.c_str(), NULL);
      }
    }
    Widget::drawLayer(args, layer);
  }
};

void ErrorTooltip::step() {
  text = error_text;
	Tooltip::step();
	// Position at bottom-right of parameter
	box.pos = errorWidget->getAbsoluteOffset(errorWidget->box.size).round();
	// Fit inside parent (copied from Tooltip.cpp)
	assert(parent);
	box = box.nudge(parent->box.zeroPos());
}

struct BasicallyWidget : ModuleWidget {
  Widget* topRightScrew;
	Widget* bottomRightScrew;
	Widget* rightHandle;
	BasicallyDisplay* codeDisplay;

  BasicallyWidget(Basically* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Basically.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    topRightScrew = createWidget<ScrewSilver>(
        Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0));
    addChild(topRightScrew);
    addChild(createWidget<ScrewSilver>(
        Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    bottomRightScrew = createWidget<ScrewSilver>(
        Vec(box.size.x - 2 * RACK_GRID_WIDTH,
            RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
    addChild(bottomRightScrew);

    codeDisplay = createWidget<BasicallyDisplay>(
      mm2px(Vec(22.0, 11.844)));
		codeDisplay->box.size = mm2px(Vec(60.0, 110.0));
		codeDisplay->setModule(module);
		addChild(codeDisplay);

    // Controls.
    // Run button/trigger/gate.
    addInput(createInputCentered<PJ301MPort>(
        mm2px(Vec(6.496, 17.698)), module, Basically::RUN_INPUT));
    // Making this a Button and not a Latch means that it pops back up
    // when you let go.
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(15.645, 17.698)),
                                             module, Basically::RUN_PARAM,
                                             Basically::RUN_LIGHT));
    RoundBlackSnapKnob* style_knob = createParamCentered<RoundBlackSnapKnob>(
        mm2px(Vec(11.07, 34.068)), module, Basically::STYLE_PARAM);
    style_knob->minAngle = -0.28f * M_PI;
    style_knob->maxAngle = 0.28f * M_PI;
    addParam(style_knob);

    // Data Inputs
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.496, 57.35)),
      module, Basically::IN1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.645, 57.35)),
      module, Basically::IN2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.496, 71.35)),
      module, Basically::IN3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.645, 71.35)),
      module, Basically::IN4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.496, 85.35)),
      module, Basically::IN5_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.645, 85.35)),
      module, Basically::IN6_INPUT));

    // The Outputs
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.496, 101.601)),
      module, Basically::OUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.645, 101.601)),
      module, Basically::OUT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.496, 115.601)),
      module, Basically::OUT3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.645, 115.601)),
      module, Basically::OUT4_OUTPUT));

    // Compilation status and error message access.
    // Want the middle of this to be at x=11.07
    ErrorWidget* display = createWidget<ErrorWidget>(mm2px(
        Vec(11.07 - 4.0, 7.844)));  // Bottom edge flush with top of code.
    display->box.size = mm2px(Vec(8.0, 4.0));
    display->module = module;
    addChild(display);

    // Set reasonable initial size of module. Will likely get updated below.
    box.size = Vec(RACK_GRID_WIDTH * 16, RACK_GRID_HEIGHT);
    // Resize bar on right.
    ModuleResizeHandle* rightHandle = new ModuleResizeHandle;
		this->rightHandle = rightHandle;
		rightHandle->module = module;
		addChild(rightHandle);

    // Set box width from loaded Module before adding to the RackWidget,
    // so modules aren't unnecessarily shoved around.
		if (module) {
      // Now set the actual size.
			box.size.x = module->width * RACK_GRID_WIDTH;
		}
  }

  void step() override {
		Basically* module = dynamic_cast<Basically*>(this->module);
    // While this is really only useful to call when the width changes,
    // I don't think it's currently worth the effort to ONLY call it then.
    // And maybe the *first* time step() is called.
		if (module) {
			box.size.x = module->width * RACK_GRID_WIDTH;
		}

    // Adjust size of area we display code in.
    // "6" here is ~4 on the left side plus ~1.5 on the right.
		codeDisplay->box.size.x = box.size.x - RACK_GRID_WIDTH * 5.5;
    // Move the right side screws to follow.
		topRightScrew->box.pos.x = box.size.x - 30;
		bottomRightScrew->box.pos.x = box.size.x - 30;
		rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;
		ModuleWidget::step();
	}

  void appendContextMenu(Menu* menu) override {
    Basically* module = dynamic_cast<Basically*>(this->module);
    // Add color choices.
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("Screen colors"));
    long long int default_colors[] = {0x00ff00000000,
                                      0xffffff000000,
                                      0xffd714000000,
                                      0xffc000000000,
                                      0x000000ffffff
                                    };
    std::string color_names[] = {"Green on Black",
                                 "White on Black",
                                 "Yellow on Black (like Notes)",
                                 "Amber on Black",
                                 "Black on White"};
    for (int i = 0; i < std::end(default_colors) - std::begin(default_colors);
         i++) {
      long long int scheme = default_colors[i];
      menu->addChild(createCheckMenuItem(color_names[i], "",
          [=]() {return scheme == module->screen_colors;},
          [=]() {module->screen_colors = scheme;}
      ));
    }
    // Options
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolPtrMenuItem("Highlight error line", "",
                                          &module->allow_error_highlight));

    // Add syntax insertions.
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel(
      "Language hints (select to insert into code)"));
    std::pair<std::string, std::string> syntax[] = {
      {"OUT1 = IN1 + IN2", "OUT1 = IN1 + IN2"},
      {"WAIT 200", "WAIT 200"},
      {"' I'm a comment.", "' I'm a comment."},
      {"IF IN1 == 0 THEN OUT1 = IN2 * IN2 END IF",
       "IF IN1 == 0 THEN\n  OUT1 = IN2 * IN2\nEND IF"},
      {"IF IN1 == 0 THEN OUT1 = IN2 * IN1 ELSE OUT1 = IN2 * IN1 END IF",
       "IF IN1 == 0 THEN\n  OUT1 = IN2 * IN1\nELSE\n  OUT1 = IN2 * IN1\nEND IF"},
      {"FOR i = 0 TO 10 foo = IN1 + i NEXT",
       "FOR i = 0 TO 10\n  foo = IN1 + i\nNEXT"},
      {"FOR i = 0 TO 10 STEP 0.2 foo = IN1 + i NEXT",
       "FOR i = 0 TO 10 STEP 0.2\n  foo = IN1 + i\nNEXT"},
      {"CONTINUE FOR", "CONTINUE FOR"},
      {"EXIT FOR", "EXIT FOR"},
      {"CONTINUE ALL", "CONTINUE ALL"},
      {"EXIT ALL", "EXIT ALL"}
    };
    MenuItem* syntax_menu = createSubmenuItem("Syntax", "",
      [=](Menu* menu) {
          for (auto line : syntax) {
            menu->addChild(createMenuItem(line.first, "",
              [=]() { codeDisplay->insertText(line.second); }
            ));
          }
      }
    );
    menu->addChild(syntax_menu);
    // Now add math functions.
    // description, inserted text.
    std::pair<std::string, std::string> math_funcs[] = {
      {"abs(x) - this number without a negative sign", "abs(IN1)"},
      {"ceiling(x) - integer value at or above x", "ceiling(IN1)"},
      {"floor(x) -  integer value at or below x", "floor(IN1)"},
      {"max(x, y) - larger of x or y", "max(IN1, -5)"},
      {"min(x, y) - smaller of x or y", "min(IN1, 5)"},
      {"mod(x, y) - remainder after dividing x by y", "mod(IN1, 1)"},
      {"pow(x, y) - x to the power of y", "pow(IN1, 0.5)"},
      {"sign(x) - -1, 0, or 1, depending on the sign of x", "sign(IN1)"},
      {"sin(x) - sine of x, which is in radians", "sin(IN1)"}
    };
    MenuItem* math_menu = createSubmenuItem("Math", "",
      [=](Menu* menu) {
          for (auto line : math_funcs) {
            menu->addChild(createMenuItem(line.first, "",
              [=]() { codeDisplay->insertText(line.second); }
            ));
          }
      }
    );
    menu->addChild(math_menu);
  }
};

Model* modelBasically = createModel<Basically, BasicallyWidget>("BASICally");
