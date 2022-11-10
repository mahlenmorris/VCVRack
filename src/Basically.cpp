#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <vector>

#include "plugin.hpp"
#include "parser/driver.hh"
#include "pcode.h"

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
    GOOD_LIGHT,  // Code compiles and can run.
    LIGHTS_LEN
  };

  std::unordered_map<std::string, OutputId> out_map { {"out1", OUT1_OUTPUT},
                                                      {"out2", OUT2_OUTPUT},
                                                      {"out3", OUT3_OUTPUT},
                                                      {"out4", OUT4_OUTPUT}
                                                    };
  std::vector<std::pair<std::string, InputId> > in_list { {"in1", IN1_INPUT},
                                                          {"in2", IN2_INPUT},
                                                          {"in3", IN3_INPUT},
                                                          {"in4", IN4_INPUT}
                                                        };

  Basically() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configButton(RUN_PARAM, "Press to run");
    configSwitch(STYLE_PARAM, 0, 3, 0, "When to run",
                 {"Always run", "Start on trigger, loop",
                  "Start on trigger, don't loop",
                  "Run when gate is open"});
    // This has distinct values.
    getParamQuantity(STYLE_PARAM)->snapEnabled = true;
    configInput(IN1_INPUT, "IN1 input.");
    configInput(IN2_INPUT, "IN2 input.");
    configInput(IN3_INPUT, "IN3 input.");
    configInput(IN4_INPUT, "IN4 input.");
    configInput(RUN_INPUT, "Trigger to start or Gate to start/stop (See Style)");
    configOutput(OUT1_OUTPUT, "OUT1 output.");
    configOutput(OUT2_OUTPUT, "OUT2 output.");
    configOutput(OUT3_OUTPUT, "OUT3 output.");
    configOutput(OUT4_OUTPUT, "OUT4 output.");
    configLight(RUN_LIGHT, "Lit when code is currently running.");
    configLight(GOOD_LIGHT, "Lit when code compiles and could run.");


    // If user decides to "bypass" the module, we can just pass IN -> OUT.
    // TODO: reconsider this Bypass behavior.
    configBypass(IN1_INPUT, OUT1_OUTPUT);

    //drv.parse("out1 := in1 / 2 \n wait 500 \n out1 := in1 * 2 wait 300");
    // Environment must be initialized with values or module will fail on start.
    environment.variables["out1"] = 0.0f;  // Default value to start.
    environment.variables["in1"] = 0.0f;  // Default value to start.
    current_line = 0;
    ticks_remaining = 0;
  }

  // If asked to, save the curve data in json for reading when loaded.
  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "text", json_stringn(text.c_str(), text.size()));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* textJ = json_object_get(rootJ, "text");
		if (textJ)
			text = json_string_value(textJ);
		dirty = true;
  }

  void ResetToProgramStart() {
    current_line = 0;
    ticks_remaining = 0;
    // TODO: Consider resetting ticks_remaining and clearing "environment"?
  }

  void UpdateOutsIfNeeded(const std::string var_name, float value) {
    auto found = out_map.find(var_name);
    if (found != out_map.end()) {
      // Limit to -10 <= x < = 10.
      outputs[found->second].setVoltage(
        std::max(-10.0f, std::min(10.0f, value)));
    }
  }


  void process(const ProcessArgs& args) override {
    Style style = STYLES[(int) params[STYLE_PARAM].getValue()];
    bool loops = (style != TRIGGER_NO_LOOP_STYLE);

    if (user_has_changed && !text.empty()) {
      user_has_changed = false;  // TODO: race condition?
      std::string lowercase;
      lowercase.resize(text.size());
      std::transform(text.begin(), text.end(),
                     lowercase.begin(), ::tolower);
      compiles = !drv.parse(lowercase);
      // Only time this light changes is when it compiles. That might not
      // always be true in the future.
      lights[GOOD_LIGHT].setBrightness(compiles ? 1.f : 0.f);
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
    // TODO: This doesn't need to run under some conditions.
    bool run_was_low = !runTrigger.isHigh();
    runTrigger.process(rescale(
        inputs[RUN_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
    bool run_button_pressed = params[RUN_PARAM].getValue() > 0.1f;
    bool asked_to_start_run = (run_button_pressed ||
                               (run_was_low && runTrigger.isHigh()));

    switch (style) {
      case ALWAYS_STYLE: {
        if (compiles) running = true;
      }
      break;
      case TRIGGER_LOOP_STYLE:
      case TRIGGER_NO_LOOP_STYLE: {
        // If we're already running, keep running.
        if (!running && asked_to_start_run) {
          running = true;
          ResetToProgramStart();
        }
      }
      break;
      case GATE: {
        running = runTrigger.isHigh() || run_button_pressed;
      }
      break;
    }

    // Update environment with current inputs.
    // If we're just waiting this tick, nothing will read the environment, so
    // no point in updating it.
    if (ticks_remaining < 2) {
      for (auto input : in_list) {
        if (inputs[input.second].isConnected()) {
          environment.variables[input.first] = inputs[input.second].getVoltage();
        }
      }
    }
    // Run the PCode vector from the current spot in it.
    bool waiting = false;

    // TODO: Could just decrement and skip this whole thing if
    // ticks_remaining > 2?
    while (running && !waiting) {
      PCode* pcode = &(pcodes[current_line]);
      switch (pcode->type) {
        case PCode::ASSIGNMENT: {
          float rhs = pcode->expr1.Compute(&environment);
          environment.variables[pcode->str1] = rhs;
          UpdateOutsIfNeeded(pcode->str1, rhs);
          current_line++;
        }
        break;
        case PCode::WAIT: {
          // TODO: Need a way for user to say "wait exactly one step"
          if (ticks_remaining > 0) {
            // We're currently running through the current wait period.
            ticks_remaining--;
            if (ticks_remaining <= 0) {
              current_line++;
            }
          } else {
            // Just arriving at this WAIT statement.
            ticks_remaining = (int) (pcode->expr1.Compute(&environment) *
                args.sampleRate / 1000.0f);
            // A "WAIT 0" (or WAIT -1!) means we should push to next line and
            // stop running for this process() call.
            if (ticks_remaining <= 0) {
              ticks_remaining = 0;
              current_line++;
              waiting = true;
            }
          }
          if (ticks_remaining > 0) {
            waiting = true;
          }
        }
        break;
        case PCode::IFNOT: {
          // All this PCode does is determine where to move current_line to.
          bool expr_val = pcode->bool1.Compute(&environment);
          if (!expr_val) {
            current_line += pcode->jump_count;
          } else {
            current_line++;
          }
        }
        break;
        case PCode::RELATIVE_JUMP: {
          // This just specifies a jump of the current_line.
          current_line += pcode->jump_count;
        }
        break;
        case PCode::FORLOOP: {
          if (state == PCode::ENTERING_FOR_LOOP) {
            pcode->limit = pcode->expr1.Compute(&environment);
            pcode->step = pcode->expr2.Compute(&environment);
          } else {
            float new_value = environment.variables[pcode->str1] + pcode->step;
            environment.variables[pcode->str1] = new_value;
            UpdateOutsIfNeeded(pcode->str1, new_value);
          }
          bool done = false;
          // If "Step" is negative, we wait until value is below limit.
          if (pcode->step >= 0.0f) {
            done = environment.variables[pcode->str1] > pcode->limit;
          } else {
            done = environment.variables[pcode->str1] < pcode->limit;
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
    lights[RUN_LIGHT].setBrightness(running ? 1.0f : 0.0f);
  }

  dsp::SchmittTrigger runTrigger;
  std::string text;
  bool dirty = false;  // Set when module changes the text (like at start).
  bool user_has_changed = true;
  bool compiles = false;
  bool running = false;
  Driver drv;
  std::vector<PCode> pcodes;  // What actually gets executed.
  Environment environment;
  unsigned int current_line;
  int ticks_remaining;
  // Some PCodes are reentrant, but with different behaviors. state helps
  // determine the behavior.
  PCode::State state;
};

struct BasicallyTextField : LedDisplayTextField {
	Basically* module;

	void step() override {
		LedDisplayTextField::step();
		if (module && module->dirty) {
			setText(module->text);
			module->dirty = false;
		}
	}

  // Return the zero-offset column.
  int GetCurrentColumn(int position) {
    std::string text = TextField::getText();
    // Work to the left until hit /n or position == 0;
    int column = 0;
    while (position > 0 && text.at(position - 1) != '\n') {
      column++;
      position--;
    }
    return column;
  }

  int CursorForColumn(int current_column, bool up) {
    int position = cursor;
    if (up) {
      // current_column is a hint how many chars to go back.
      position -= current_column;
      if (position == 0) {
        // At top of buffer, can't go up.
        return cursor;
      }
      position--;  // Now positioned before the previous line's '\n'
      int line_length = GetCurrentColumn(position);
      if (line_length <= current_column) {
        // If previous line is too short, just stay here.
        return position;
      }
      return position - (line_length - current_column);
    } else {
      // Down one line.
      int text_size = text.size();
      bool on_next_line;
      for (; position < text_size; position++) {
        if (text.at(position) == '\n') {
          on_next_line = true;
          break;
        }
      }
      if (!on_next_line) {
        // Must be on last line, cannot go down.
        return cursor;
      }
      position++;  // Now at column zero of next line.
      int goal = std::min(position + current_column, text_size);
      // We want to advance current_column chars, but not go past any '\n'.
      while (position <= goal) {
        // Cannot call .at(position) when position == text_size.
        if ((position == text_size) || text.at(position) == '\n') {
          return position;
        }
        if (position == goal) return position;
        position++;
      }
    }
    // Shouldn't get here, but this won't crash anything.
    return cursor;
  }

  // So we can handle up and down keys.
  void onSelectKey(const SelectKeyEvent& e) override {
    if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
      // Up (placeholder)
  		if (e.key == GLFW_KEY_UP) {
        // Move to same column, in previous line.
        int column = GetCurrentColumn(cursor);
        cursor = CursorForColumn(column, true);
        if (!(e.mods & GLFW_MOD_SHIFT)) {
  				selection = cursor;  // Otherwise we select the line.
  			}
  			e.consume(this);
  		}
  		// Down (placeholder)
  		if (e.key == GLFW_KEY_DOWN) {
        // Move to same column, in next line.
        int column = GetCurrentColumn(cursor);
        cursor = CursorForColumn(column, false);
        if (!(e.mods & GLFW_MOD_SHIFT)) {
  				selection = cursor;  // Otherwise we select the line.
  			}
  			e.consume(this);
  		}
    }
    TextField::onSelectKey(e);
  }

  // User has updated the text.
	void onChange(const ChangeEvent& e) override {
		if (module) {
			module->text = getText();
      module->user_has_changed = true;
    }
	}
};

struct BasicallyDisplay : LedDisplay {
	void setModule(Basically* module) {
		BasicallyTextField* textField = createWidget<BasicallyTextField>(Vec(0, 0));
		textField->box.size = box.size;
		textField->multiline = true;
		textField->module = module;
		addChild(textField);
	}
};

struct BasicallyWidget : ModuleWidget {
  BasicallyWidget(Basically* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Basically.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(
        Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(
        Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(
        Vec(box.size.x - 2 * RACK_GRID_WIDTH,
            RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    BasicallyDisplay* codeDisplay = createWidget<BasicallyDisplay>(
      mm2px(Vec(0.360, 11.844)));
		codeDisplay->box.size = mm2px(Vec(60.0, 75.0));
		codeDisplay->setModule(module);
		addChild(codeDisplay);

    // Controls.
    // Run button/trigger/gate.
    addInput(createInputCentered<PJ301MPort>(
        mm2px(Vec(6.496, 103.24)), module, Basically::RUN_INPUT));
    // Making this a Button and not a Latch means that it pops back up
    // when you let go.
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(6.496, 90.781)),
                                             module, Basically::RUN_PARAM,
                                             Basically::RUN_LIGHT));
    RoundBlackSnapKnob* style_knob = createParamCentered<RoundBlackSnapKnob>(
        mm2px(Vec(6.496, 114.863)), module, Basically::STYLE_PARAM);
    style_knob->minAngle = -0.28f * M_PI;
    style_knob->maxAngle = 0.28f * M_PI;
    addParam(style_knob);

    // Data Inputs
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.246, 103.24)),
      module, Basically::IN1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.073, 103.24)),
      module, Basically::IN2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.9, 103.24)),
      module, Basically::IN3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(52.728, 103.24)),
      module, Basically::IN4_INPUT));

    // The Outputs
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(23.246, 118.574)),
      module, Basically::OUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33.073, 118.574)),
      module, Basically::OUT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.9, 118.574)),
      module, Basically::OUT3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(52.728, 118.574)),
      module, Basically::OUT4_OUTPUT));

    // Lights
    addChild(createLightCentered<MediumLight<GreenLight>>(
      mm2px(Vec(42.9, 90.781)), module, Basically::GOOD_LIGHT));

  }
};

Model* modelBasically = createModel<Basically, BasicallyWidget>("BASICally");
