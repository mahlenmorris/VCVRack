#include <algorithm>
#include <vector>

#include "plugin.hpp"
#include "parser/driver.hh"

// While I should probably replace "point" with Vec, this struct comes
// with less baggage.
struct point {
  float x;
  float y;
};

enum LineType {
  STEPS_LINETYPE,
  LINES_LINETYPE,
  SMOOTHSTEP_LINETYPE
};

LineType LINES_DOOMED[] = {
  STEPS_LINETYPE,
  LINES_LINETYPE,
  SMOOTHSTEP_LINETYPE
};

const int DISPLAY_POINT_COUNT = 100;

struct Basically : Module {
  enum ParamId {
    OFFSET_PARAM,
    LINETYPE_PARAM,
    RESET_PARAM,
    DRIFT_PARAM,
    SCALE_PARAM,
    SEGMENTS_PARAM,
    ENDPOINTS_PARAM,
    X_SCALE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    RESET_INPUT,
    DRIFT_INPUT,
    DOMAIN_INPUT,
    SCALE_INPUT,
    X_SCALE_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    RANGE_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    OFFSET_LIGHT,
    RESET_LIGHT,
    DRIFT_LIGHT,
    ENDPOINTS_LIGHT,
    LIGHTS_LEN
  };

  Basically() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(SCALE_PARAM, 0.0f, 10.0f, 1.0f,
                "Plus the Total Drift input value = the maximum total drift distance per drift event");
    configParam(X_SCALE_PARAM, 0.0f, 10.0f, 10.0f,
                "Plus the X Drift input value = the maximum x_axis drift distance per drift event");
    // TODO: make these actual integers.
    configParam(SEGMENTS_PARAM, 1.0f, 32.0f, 8.0f,
                "Number of sections in the curve upon next RESET event");
    // This is really an integer.
    getParamQuantity(SEGMENTS_PARAM)->snapEnabled = true;

    configButton(RESET_PARAM, "Press to reset curve to flat");
    configButton(DRIFT_PARAM, "Press to drift once");
    // TODO: Shouldn't Unipolar reset to all zeroes?
    configSwitch(OFFSET_PARAM, 0, 1, 0, "Offset",
                 {"Bipolar (-5V - +5V)", "Unipolar (0V - 10V)"});
    configSwitch(LINETYPE_PARAM, 0, 2, 1, "Line Type",
                 {"Steps", "Lines", "Lines + Smoothstep"});
    // This has distinct values.
    getParamQuantity(LINETYPE_PARAM)->snapEnabled = true;

    configSwitch(ENDPOINTS_PARAM, 0, 1, 0, "Endpoints are",
                 {"Fixed", "Drifting"});

    configInput(RESET_INPUT, "Line is reset to flat when a trigger enters");
    configInput(DRIFT_INPUT, "The curve drifts when a trigger enters");
    configInput(DOMAIN_INPUT, "The X position on the curve");
    configInput(SCALE_INPUT,
                "Added to knob value -> the maximum total drift distance per drift event");
    configInput(X_SCALE_INPUT,
                "Added to knob value -> the maximum x-axis drift distance per drift event");
    configOutput(RANGE_OUTPUT, "The Y position on the curve at IN.");

    // If user decides to "bypass" the module, we can just pass IN -> OUT.
    configBypass(DOMAIN_INPUT, RANGE_OUTPUT);

    drv.parse("foo := in1 * 1 /2 \n out1 := foo * (0 - 1)");
    environment.variables["out1"] = 0.0f;  // Default value to start.
    current_line = 0;
  }

  // If asked to, save the curve data in json for reading when loaded.
  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    if (saveCurveInRack) {
      json_object_set_new(rootJ, "saveCurve", json_integer(1));
      json_t* point_array_json = json_array();
      for (point p : points) {
        json_t* point_json = json_array();
        json_array_append_new(point_json, json_real(p.x));
        json_array_append_new(point_json, json_real(p.y));
        json_array_append_new(point_array_json, point_json);
      }
      json_object_set(rootJ, "points", point_array_json);
      json_decref(point_array_json);
      json_object_set_new(rootJ, "start_y", json_real(start_point.y));
      json_object_set_new(rootJ, "end_y", json_real(end_point.y));
    }
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* saveJ = json_object_get(rootJ, "saveCurve");
    if (saveJ) {
      saveCurveInRack = json_integer_value(saveJ) == 1;
    }
    if (saveCurveInRack) {  // load it.
      json_t* pointsJ = json_object_get(rootJ, "points");
      if (pointsJ) {
        // 100 is far more points than we'll save; just avoiding the
        // possibility of an infinite loop.
        for (size_t i = 0; i < 100; i++) {
          json_t* point_json = json_array_get(pointsJ, i);
          if (point_json) {
            json_t* xJ = json_array_get(point_json, 0);
            json_t* yJ = json_array_get(point_json, 1);
            if (xJ && yJ) {
              point new_point;
              new_point.x = json_real_value(xJ);
              new_point.y = json_real_value(yJ);
              loaded_points.push_back(new_point);
            }
          } else {
            break;  // No more points to read.
          }
        }
      }
      json_t* startJ = json_object_get(rootJ, "start_y");
      if (startJ) {
        loaded_start_y = json_real_value(startJ);
      }
      json_t* endJ = json_object_get(rootJ, "end_y");
      if (endJ) {
        loaded_end_y = json_real_value(endJ);
      }
    }
  }

  void process(const ProcessArgs& args) override {
    // I'd have preferred to initialize 'points' in the constructor,
    // but it seems that the values of knobs (i.e., params) aren't
    // available in the constructor. So I wait until here, when they
    // are definitely set.
    if (!initialized) {
      initialized = true;
    }

    // Some lights are lit by triggers; these enable them to be
    // lit long enough to be seen by humans.
    if (drifting_light_countdown > 0) {
      drifting_light_countdown--;
    }
    if (reset_light_countdown > 0) {
      reset_light_countdown--;
    }

    int type_knob = params[LINETYPE_PARAM].getValue();
    LineType line_type = LINES_DOOMED[type_knob];
    if (line_type != prev_line_type) {
      prev_line_type = line_type;
    }

    bool endpoints_drift = params[ENDPOINTS_PARAM].getValue() > 0;
    // Test the Reset button and signal.
    bool reset_was_low = !resetTrigger.isHigh();
    resetTrigger.process(rescale(
        inputs[RESET_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
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

    // Compute the current value of OUT.
    // TODO: Use notion of line pointer.
    // Update environment with current inputs.
    if (inputs[DOMAIN_INPUT].isConnected()) {
      environment.variables["in1"] = inputs[DOMAIN_INPUT].getVoltage();
    }
    // Run the line.
    // TODO: does this belong in Line class?
    bool waiting = false;
    while (!waiting) {
      switch (drv.lines[current_line].type) {
        case Line::ASSIGNMENT: {
          Line* assignment = &(drv.lines[current_line]);
          environment.variables[assignment->str1] =
              assignment->expr1.Compute(&environment);
          current_line++;
          if (current_line >= drv.lines.size()) {
            current_line = 0;
            waiting = true;  // Implicit WAIT at end of program.
          }
        }
        break;
        case Line::WAIT:
          break;
      }
    }

    // Limit to -10 <= x < = 10.
    float out1 = std::max(-10.0f, std::min(10.0f, environment.variables["out1"]));
    outputs[RANGE_OUTPUT].setVoltage(out1);

    // Lights.
    lights[RESET_LIGHT].setBrightness(
        reset || reset_light_countdown > 0 ? 1.0f : 0.0f);
    lights[ENDPOINTS_LIGHT].setBrightness(endpoints_drift);
    lights[DRIFT_LIGHT].setBrightness(
        drifting_light_countdown > 0 ? 1.0f : 0.0f);
  }

  dsp::SchmittTrigger driftTrigger, resetTrigger;
  point start_point = {0.0f, 5.0f};  // Y value depends on OFFSET type.
  point end_point = {10.0f, 5.0f};  // Y value depends on OFFSET type.
  std::vector<point> points;
  point display_points[DISPLAY_POINT_COUNT];

  // Flipped each drift to prevent unidirectional bias.
  bool left_to_right = true;

  // Solely so we start with the right number of points.
  bool initialized = false;

  // Make sure we only drift once when DRIFT button is pressed.
  bool drift_button_pressed = false;

  // Keeps lights lit long enough to see.
  int drifting_light_countdown = 0;
  int reset_light_countdown = 0;

  // Need to update display if LineType changes.
  LineType prev_line_type = STEPS_LINETYPE;

  // Set by context menu.
  bool saveCurveInRack = false;
  std::vector<point> loaded_points;
  float loaded_start_y, loaded_end_y;
  Driver drv;
  Environment environment;
  unsigned int current_line;
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

    // OFST
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(37.224, 48.0)),
                                             module, Basically::OFFSET_PARAM,
                                             Basically::OFFSET_LIGHT));
    // ENDS
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(37.224, 64.0)),
                                             module, Basically::ENDPOINTS_PARAM,
                                             Basically::ENDPOINTS_LIGHT));

    // Line Count.
    addParam(createParamCentered<RoundBlackKnob>(
         mm2px(Vec(37.224, 80.0)), module, Basically::SEGMENTS_PARAM));
    // Line Style
    RoundBlackSnapKnob* line_knob = createParamCentered<RoundBlackSnapKnob>(
         mm2px(Vec(37.224, 96.0)), module, Basically::LINETYPE_PARAM);
    line_knob->minAngle = -0.28f * M_PI;
    line_knob->maxAngle = 0.28f * M_PI;
    addParam(line_knob);

    // X Drift.
    // Input.
    addInput(createInputCentered<PJ301MPort>(
        mm2px(Vec(8.024, 48.0)), module, Basically::X_SCALE_INPUT));
    // Knob.
    addParam(createParamCentered<RoundBlackKnob>(
         mm2px(Vec(22.624, 48.0)), module, Basically::X_SCALE_PARAM));

    // Total Drift.
    // Input.
    addInput(createInputCentered<PJ301MPort>(
        mm2px(Vec(8.024, 64.0)), module, Basically::SCALE_INPUT));
    // Knob.
    addParam(createParamCentered<RoundBlackKnob>(
         mm2px(Vec(22.624, 64.0)), module, Basically::SCALE_PARAM));

    // Commands from user/system.
    // Drift
    addInput(createInputCentered<PJ301MPort>(
        mm2px(Vec(8.024, 80.0)), module, Basically::DRIFT_INPUT));
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(22.624, 80.0)),
                                             module, Basically::DRIFT_PARAM,
                                             Basically::DRIFT_LIGHT));

    // Reset
    addInput(createInputCentered<PJ301MPort>(
        mm2px(Vec(8.024, 96.0)), module, Basically::RESET_INPUT));
    // Making this a Button and not a Latch means that it pops back up
    // when you let go.
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(22.624, 96.0)),
                                             module, Basically::RESET_PARAM,
                                             Basically::RESET_LIGHT));

    // Input
    addInput(createInputCentered<PJ301MPort>(
        mm2px(Vec(13.905, 112.000)), module, Basically::DOMAIN_INPUT));

    // The Output
    addOutput(createOutputCentered<PJ301MPort>(
        mm2px(Vec(27.797, 112.000)), module, Basically::RANGE_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    Basically* module = dynamic_cast<Basically*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolPtrMenuItem("Save curve in rack", "",
                                          &module->saveCurveInRack));
  }
};

Model* modelBasically = createModel<Basically, BasicallyWidget>("BASICally");
