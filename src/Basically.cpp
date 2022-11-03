#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <vector>

#include "plugin.hpp"
#include "parser/driver.hh"
#include "pcode.h"

struct Basically : Module {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    IN1_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    OUT1_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  std::unordered_map<std::string, OutputId> out_map { {"out1", OUT1_OUTPUT}
                                                    };

  Basically() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(IN1_INPUT, "IN1 input.");
    configOutput(OUT1_OUTPUT, "OUT1 output.");

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

  void process(const ProcessArgs& args) override {
    // I'd have preferred to initialize 'points' in the constructor,
    // but it seems that the values of knobs (i.e., params) aren't
    // available in the constructor. So I wait until here, when they
    // are definitely set.
    if (!initialized) {
      initialized = true;
    }
    // If the program has just recompiled, certain optimizations may no
    // longer apply. "recompiled" tells us that the program is new.
    bool recompiled = false;
    if (user_has_changed && !text.empty()) {
      user_has_changed = false;  // TODO: race condition?
      std::string lowercase;
      lowercase.resize(text.size());
      std::transform(text.begin(), text.end(),
                     lowercase.begin(), ::tolower);
      ok_to_run = !drv.parse(lowercase);
      if (ok_to_run) {
        PCode::LinesToPCode(drv.lines, &pcodes);
        recompiled = true;
      }
    }

    // Update environment with current inputs.
    // TODO: avoid calling this if ticks_remaining > 1?
    if (inputs[IN1_INPUT].isConnected()) {
      environment.variables["in1"] = inputs[IN1_INPUT].getVoltage();
    }
    // Run the PCode vector from the current spot in it.
    bool waiting = false;

    //INFO("current_line=%i, ticks_remaining=%i", current_line, ticks_remaining);

    if (recompiled) {
      current_line = 0;
    }
    while (ok_to_run && !waiting) {
      switch (pcodes[current_line].type) {
        case PCode::ASSIGNMENT: {
          PCode* assignment = &(pcodes[current_line]);
          float rhs = assignment->expr1.Compute(&environment);
          environment.variables[assignment->str1] = rhs;
          auto found = out_map.find(assignment->str1);
          if (found != out_map.end()) {
            // Limit to -10 <= x < = 10.
            outputs[found->second].setVoltage(
              std::max(-10.0f, std::min(10.0f, rhs)));
          }
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
            PCode* wait = &(pcodes[current_line]);
            ticks_remaining = (int) (wait->expr1.Compute(&environment) *
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
          PCode* ifnot = &(pcodes[current_line]);
          bool expr_val = ifnot->bool1.Compute(&environment);
          if (!expr_val) {
            current_line += ifnot->jump_count;
          } else {
            current_line++;
          }
        }
        break;
      }
      if (current_line >= pcodes.size()) {
        current_line = 0;
        waiting = true;  // Implicit WAIT at end of program.
      }
    }

    // Lights.
  }

  // Solely so we start with the right number of points.
  bool initialized = false;

  std::string text;
  bool dirty = false;  // Set when module changes the text (like at start).
  bool user_has_changed = false;
  bool ok_to_run = false;
  Driver drv;
  std::vector<PCode> pcodes;  // What actually gets executed.
  Environment environment;
  unsigned int current_line;
  int ticks_remaining;
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

  // User has updated the text.
	void onChange(const ChangeEvent& e) override {
		if (module)
			module->text = getText();
      module->user_has_changed = true;
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
		codeDisplay->box.size = mm2px(Vec(45.0, 84.0));
		codeDisplay->setModule(module);
		addChild(codeDisplay);

    // Input
    addInput(createInputCentered<PJ301MPort>(
        mm2px(Vec(13.905, 112.000)), module, Basically::IN1_INPUT));

    // The Output
    addOutput(createOutputCentered<PJ301MPort>(
        mm2px(Vec(27.797, 112.000)), module, Basically::OUT1_OUTPUT));
  }
};

Model* modelBasically = createModel<Basically, BasicallyWidget>("BASICally");