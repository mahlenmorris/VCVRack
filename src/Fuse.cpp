#include "plugin.hpp"

struct Fuse : Module {
  enum ParamId {
    COUNT_PARAM,
    STYLE_PARAM,
		RESET_PARAM,
		TRIGGER_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    RESET_INPUT,
    TRIGGER_INPUT,
    MAIN_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    BLOWN_OUTPUT,
    MAIN_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    RESET_LIGHT,
    TRIGGER_LIGHT,
    LIGHTS_LEN
  };

  Fuse() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch(STYLE_PARAM, 0, 3, 0,
                 "Value of OUT as count -> LIMIT",
                 {"BLOW CLOSED (IN -> 0.0)",
                  "BLOW OPEN (0.0 -> IN)",
                  "NARROW (IN * (1 - count/LIMIT) -> 0.0)",
                  "WIDEN (IN * (count/LIMIT) -> IN)"
                 }
                );
    // Knob should snap to distinct values.
    getParamQuantity(STYLE_PARAM)->snapEnabled = true;

    configParam(COUNT_PARAM, 1, 1000, 10,
        "Number of TRIGGER events until fuse blows (see STYLE)");
    // This is really an integer.
    getParamQuantity(COUNT_PARAM)->snapEnabled = true;

    configInput(RESET_INPUT, "Count is reset to zero when a trigger enters");
    configButton(RESET_PARAM, "Press to reset count of triggers to zero");

    configInput(TRIGGER_INPUT, "Adds one to the count each time a trigger enters");
    configButton(TRIGGER_PARAM, "Press to add one to the count");

    configOutput(BLOWN_OUTPUT, "Outputs a trigger when count hits LIMIT");

    configInput(MAIN_INPUT, "In");
    configOutput(MAIN_OUTPUT, "Out");

    // If user decides to "bypass" the module, we can just pass IN -> OUT.
    configBypass(MAIN_INPUT, MAIN_OUTPUT);

    count = 0;
  }

  // Overriding as a precaution. In case future versions need to save
  // menued config data, previous versions will already have a 'data' object.
  // See https://community.vcvrack.com/t/how-to-initialize-new-params-when-and-old-patch-is-loaded
  // for reasoning.
  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    return rootJ;
  }

  // Returns completion from 0.0 -> 1.0.
  float getCompletion() {
    int limit = params[COUNT_PARAM].getValue();
    int effective_count = count > limit ? limit : count;
    if (limit > 0) {
      return 1.0f * effective_count / limit;
    } else {
      return 0.0f;
    }
  }

  int getStyle() {
    return params[STYLE_PARAM].getValue();
  }

  void process(const ProcessArgs& args) override {
    // Some lights are lit by triggers; these enable them to be
    // lit long enough to be seen by humans.
    if (trigger_light_countdown > 0) {
      trigger_light_countdown--;
    }
    if (reset_light_countdown > 0) {
      reset_light_countdown--;
    }

    // Test the RESET button and input.
    bool reset_was_low = !resetTrigger.isHigh();
    resetTrigger.process(rescale(
        inputs[RESET_INPUT].getVoltage(), 0.1f, 2.0f, 0.0f, 1.0f));
    if (reset_was_low && resetTrigger.isHigh()) {
      // Flash the reset light for a tenth of second.
      // Compute how many samples to show the light.
      reset_light_countdown = std::floor(args.sampleRate / 10.0f);
    }
    // Note that we don't bother to set reset_light_countdown when the user
    // presses the button; we just light up the button while it's
    // being pressed.
    bool reset = (params[RESET_PARAM].getValue() > 0.1f) ||
      (reset_was_low && resetTrigger.isHigh());
    if (reset) {
      count = 0;
    }

    // Test the TRIGGER button and input.
    // TODO: should TRIGGER in UI go back to COUNTER?
    bool trigger_was_low = !counterTrigger.isHigh();
    counterTrigger.process(rescale(inputs[TRIGGER_INPUT].getVoltage(),
                                   0.1f, 2.0f, 0.0f, 1.0f));
    bool trigger_from_input = trigger_was_low && counterTrigger.isHigh();

    // We only want one trigger from a button press.
    bool trigger_from_button = false;
    if (params[TRIGGER_PARAM].getValue() > 0.1f) {
      if (!trigger_button_pressed) {
        trigger_from_button = true;
        trigger_button_pressed = true;
      }
    } else {
      trigger_button_pressed = false;
    }

    int limit = params[COUNT_PARAM].getValue();

    // Determine whether or not the Fuse has blown, as certain behaviors
    // change with that fact. Note that Blown-ness can be changed by adding
    // a TRIGGER event or by lowering the LIMIT.
    bool blown = (count >= limit);
    if (!blown && (trigger_from_input || trigger_from_button)) {
	    count += 1;
      // Flash the TRIGGER light for a tenth of second.
      // Compute how many samples to show the light.
      // Note that, in contrast to RESET, we do set a timer on the TRIGGER
      // light; because we want to convey that TRIGGER does ONE TRIGGER per
      // press, but RESET is just as reset no matter how long you hold it.
      trigger_light_countdown = std::floor(args.sampleRate / 10.0f);
	    if (count >= limit) {
	      blown = true;
	    }
    }
    if (!was_blown && blown) {
      blownGenerator.trigger(1e-3f);
    }
    was_blown = blown;

    int style = getStyle();
    int effective_count = count > limit ? limit : count;
    float out_voltage = 0.0f;

    if (blown) {
      if (style != 0 && style != 2) {
        // styles 0, 2 don't require us to have IN value.
        out_voltage = inputs[MAIN_INPUT].getVoltage();
      }
    } else {
      if (style != 1) {
        float in_voltage = inputs[MAIN_INPUT].getVoltage();
        switch (style) {
          case 0:
            out_voltage = in_voltage;
            break;
          case 2:
            out_voltage = in_voltage * (1.0f -
              (1.0f * effective_count / limit));
            break;
          case 3:
            out_voltage = in_voltage * (1.0f * effective_count / limit);
            break;
        }
      }
    }
    outputs[MAIN_OUTPUT].setVoltage(out_voltage);

    // Set output for BLOWN.
    outputs[BLOWN_OUTPUT].setVoltage(
        blownGenerator.process(args.sampleTime) ? 10.0f : 0.0f);

    // Button Lights.
    lights[RESET_LIGHT].setBrightness(
      reset || reset_light_countdown > 0 ? 1.0f: 0.0f);
    lights[TRIGGER_LIGHT].setBrightness(
      trigger_light_countdown > 0 ? 1.0f: 0.0f);
  }

  dsp::SchmittTrigger counterTrigger, resetTrigger;

  // On previous step, were we blown?
  bool was_blown = false;

  // Count of events observed since last reset.
  int count;

  dsp::PulseGenerator blownGenerator;

  // Make sure we only trigger once when TRIGGER button is pressed.
  bool trigger_button_pressed = false;

  // Keeps lights on buttons lit long enough to see.
  int reset_light_countdown = 0;
  int trigger_light_countdown = 0;
};

struct FuseDisplay : Widget {
  Fuse* module;
  std::string fontPath;
  int ticks_to_keep_text_visible = 0;

  FuseDisplay() {
    fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
  }

  void onHover(const HoverEvent & e) override {
    ticks_to_keep_text_visible = 5;  // Arbitrary, but seems to be enough.
  }

  // Given two points that define a line, find y for the given x.
  float find_y(Vec point_0, Vec point_1, float x) {
    // Need to be careful if x1 == x0.
    float x_diff = std::max(point_1.x - point_0.x, 0.00001f);
    float a = (point_1.y - point_0.y) / x_diff;
    float b = point_0.y - a * point_0.x;
    return a * x + b;
  }

  void drawLayer(const DrawArgs& args, int layer) override {
    if ((layer == 1) && (module) /* TODO: ??? && module->initialized */) {
      Rect r = box.zeroPos(); // .shrink(Vec(4, 5));  // TODO: ???
      Vec bounding_box = r.getBottomRight();

      float completion = module->getCompletion();
      int style = module->getStyle();

      // Draw the completed part in green/yellow/red.
      // We track this color, in case there will be text on top of it.
      NVGcolor main_color = SCHEME_GREEN;
      if (completion > 0.0f) {
        nvgBeginPath(args.vg);
        // In the case of style 0 or 1, that's a rectangle.
        // For style 2, it's a triangle tapering off to the right.
        // To not touch the edges, we squinch in 0.5 units on all sides.
        switch (style) {
          case 0:
            nvgRect(args.vg, 0.5, 0.5,
                    bounding_box.x * completion - 1.0f, bounding_box.y - 1.0f);
            if (completion < 0.7f) {
              main_color = SCHEME_GREEN;
            } else if (completion < 1.0f) {
              main_color = SCHEME_YELLOW;
            } else {
              main_color = SCHEME_RED;
            }
            break;
          case 1:
            nvgRect(args.vg, 0.5, 0.5,
                    bounding_box.x * completion - 1.0f, bounding_box.y - 1.0f);
            if (completion < 0.7f) {
              main_color = SCHEME_RED;
            } else if (completion < 1.0f) {
              main_color = SCHEME_YELLOW;
            } else {
              main_color = SCHEME_GREEN;
            }
            break;
          case 2:
            nvgMoveTo(args.vg, 0.5, 0.5);
            nvgLineTo(args.vg, bounding_box.x - 0.5f, bounding_box.y / 2.0f);
            nvgLineTo(args.vg, 0.5, bounding_box.y - 0.5);
            nvgClosePath(args.vg);
            main_color = color::plus(
              color::mult(SCHEME_RED, completion),
              color::mult(SCHEME_GREEN, 1 - completion));
            break;
          case 3:
            nvgMoveTo(args.vg, 0.5, bounding_box.y / 2.0f);
            nvgLineTo(args.vg, bounding_box.x - 0.5f, 0.5);
            nvgLineTo(args.vg, bounding_box.x - 0.5f, bounding_box.y - 0.5);
            nvgClosePath(args.vg);
            main_color = color::plus(
              color::mult(SCHEME_GREEN, completion),
              color::mult(SCHEME_RED, 1 - completion));
            break;
        }
        nvgFillColor(args.vg, main_color);
        nvgFill(args.vg);
      }
      if (completion < 1.0f) {
        // Fill in the rest with gray.
        float cut_x = bounding_box.x * completion - 0.5f;
        nvgBeginPath(args.vg);
        switch (style) {
          case 0:
          case 1:
            // In the case of style 0 or 1, that's a rectangle.
            nvgRect(args.vg, cut_x, 0.5,
                             bounding_box.x * (1.0 - completion),
                             bounding_box.y - 1.0f);
            break;
          case 2:
            {
              Vec upper_left(0.5, 0.5),
                  right(bounding_box.x - 0.5f, bounding_box.y / 2.0f),
                  lower_left(0.5, bounding_box.y - 0.5);
              nvgMoveTo(args.vg, right.x, right.y);  // Start at the tip.
              nvgLineTo(args.vg, cut_x, find_y(upper_left, right, cut_x));
              nvgLineTo(args.vg, cut_x, find_y(lower_left, right, cut_x));
              nvgClosePath(args.vg);
              break;
            }
          case 3:
            {
              Vec left(0.5, bounding_box.y / 2.0f),
                  upper_right(bounding_box.x - 0.5f, 0.5),
                  lower_right(bounding_box.x - 0.5f, bounding_box.y - 0.5);
              nvgMoveTo(args.vg, upper_right.x, upper_right.y);
              nvgLineTo(args.vg, lower_right.x, lower_right.y);
              nvgLineTo(args.vg, cut_x, find_y(left, lower_right, cut_x));
              nvgLineTo(args.vg, cut_x, find_y(left, upper_right, cut_x));
              nvgClosePath(args.vg);
              break;
            }
        }
        nvgFillColor(args.vg, color::mult(color::WHITE, 0.5f));
        nvgFill(args.vg);
      }

      if (ticks_to_keep_text_visible > 0) {
        ticks_to_keep_text_visible--;
      }

      if (ticks_to_keep_text_visible > 0) {
        std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
        if (font) {
          // WHITE is really hard to read on YELLOW.
          nvgFillColor(args.vg,
              color::isEqual(main_color, SCHEME_YELLOW) ? color::BLACK
                                                        : color::WHITE);
          nvgFontSize(args.vg, 13);
          nvgFontFaceId(args.vg, font->handle);
          nvgTextLetterSpacing(args.vg, -2);

          std::string text = std::to_string(module->count);
          // Place on the line just off the left edge.
          nvgText(args.vg, 1, bounding_box.y / 2.0 + 4, text.c_str(), NULL);

          text = std::to_string(static_cast<int>(
            floor(completion * 100))) + "%";
          // Place on the line just off the left edge.
          float backspace = text.length() * 4.5f + 2;
          nvgText(args.vg, bounding_box.x - backspace, bounding_box.y / 2.0 + 4,
                  text.c_str(), NULL);
        }
      }
    }
    Widget::drawLayer(args, layer);
  }
};

struct FuseWidget : ModuleWidget {
  FuseWidget(Fuse* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Fuse.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(
	      Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(
	      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(
	      Vec(box.size.x - 2 * RACK_GRID_WIDTH,
	          RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<RoundBlackKnob>(
        mm2px(Vec(20.971, 48.0)), module, Fuse::COUNT_PARAM));

    // Screen at the top.
    FuseDisplay* display = createWidget<FuseDisplay>(
      mm2px(Vec(1.240, 30.5)));
    display->box.size = mm2px(Vec(28.0, 4.0));
    display->module = module;
    addChild(display);

    // style as count increase.
    RoundBlackSnapKnob* style_knob = createParamCentered<RoundBlackSnapKnob>(
        mm2px(Vec(8.024, 48.0)), module, Fuse::STYLE_PARAM);
    style_knob->minAngle = -0.28f * M_PI;
    style_knob->maxAngle = 0.28f * M_PI;
    addParam(style_knob);

    // Trigger
    addInput(createInputCentered<PJ301MPort>(
	      mm2px(Vec(8.024, 64.0)), module, Fuse::TRIGGER_INPUT));
        // Making this a Button and not a Latch means that it pops back up
        // when you let go.
        addParam(createLightParamCentered<VCVLightButton<
                 MediumSimpleLight<WhiteLight>>>(mm2px(Vec(20.971, 64.0)),
                                                 module, Fuse::TRIGGER_PARAM,
                                                 Fuse::TRIGGER_LIGHT));

    // Reset
    addInput(createInputCentered<PJ301MPort>(
	      mm2px(Vec(8.024, 80.0)), module, Fuse::RESET_INPUT));
        // Making this a Button and not a Latch means that it pops back up
        // when you let go.
        addParam(createLightParamCentered<VCVLightButton<
                 MediumSimpleLight<WhiteLight>>>(mm2px(Vec(20.971, 80.0)),
                                                 module, Fuse::RESET_PARAM,
                                                 Fuse::RESET_LIGHT));

    addOutput(createOutputCentered<PJ301MPort>(
	      mm2px(Vec(15.24, 96.0)), module, Fuse::BLOWN_OUTPUT));

    addInput(createInputCentered<PJ301MPort>(
	      mm2px(Vec(8.024, 112.0)), module, Fuse::MAIN_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(
	      mm2px(Vec(20.971, 112.0)), module, Fuse::MAIN_OUTPUT));
  }
};

Model* modelFuse = createModel<Fuse, FuseWidget>("Fuse");
