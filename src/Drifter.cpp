#include <algorithm>
#include <vector>

#include "plugin.hpp"

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

LineType LINES[] = {
  STEPS_LINETYPE,
  LINES_LINETYPE,
  SMOOTHSTEP_LINETYPE
};

const int DISPLAY_POINT_COUNT = 100;

struct Drifter : Module {
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

  Drifter() {
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

    configButton(RESET_PARAM,
                 "Press to reset curve to initial shape (see menu)");
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

    configInput(RESET_INPUT,
                "Line is reset to initial shape when a trigger enters");
    configInput(DRIFT_INPUT, "The curve drifts when a trigger enters");
    configInput(DOMAIN_INPUT, "The X position on the curve");
    configInput(SCALE_INPUT,
                "Added to knob value -> the maximum total drift distance per drift event");
    configInput(X_SCALE_INPUT,
                "Added to knob value -> the maximum x-axis drift distance per drift event");
    configOutput(RANGE_OUTPUT, "The Y position on the curve at IN.");

    // If user decides to "bypass" the module, we can just pass IN -> OUT.
    configBypass(DOMAIN_INPUT, RANGE_OUTPUT);
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
    json_object_set_new(rootJ, "reset_shape", json_integer(reset_shape));
    json_object_set_new(rootJ, "reset_type", json_integer(reset_type));
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
    json_t* shapeJ = json_object_get(rootJ, "reset_shape");
    if (shapeJ) {
      reset_shape = json_integer_value(shapeJ);
    }
    json_t* typeJ = json_object_get(rootJ, "reset_type");
    if (typeJ) {
      reset_type = json_integer_value(typeJ);
    }
  }

  void reset_points(bool startup) {
    static std::function<float(float, float, bool)> reset_functions[] = {
      [](float x, float w, bool uni) { return 0.0 + (uni ? 0.0f : 5.0f);},  // Zero
      [](float x, float w, bool uni)
         { return 5.0f + 5.0f * sin(3.1415927f * (x + w) / 5.0f);},  // Sine.
      [](float x, float w, bool uni)
        { float xw = x + w;    // Triangle.
          if (xw <= 2.5f) {
             return 5.0f + 2.0f * xw;
          } else if (xw <= 7.5f) {
            return 5.0f + -2.0f * (xw - 5.0f);
          } else if (xw <= 12.5) {
            return 5.0f + 2.0f * (xw - 10.f);
          } else {
            return 5.0f + -2.0f * (xw - 15.0f);
          }
        },
      [](float x, float w, bool uni)
        { float xw = x + w;    // Rising Saw.
          if (xw <= 10.0f) {
             return xw;
          } else {
            return (xw - 10.0f);
          }
        },
      [](float x, float w, bool uni)
        { float xw = x + w;    // Falling Saw.
          if (xw <= 10.0f) {
             return 10.0f - xw;
          } else {
            return 10.0f - (xw - 10.0f);
          }
        },
      [](float x, float w, bool uni)
        { float xw = x + w;    // Square.
          if (xw <= 5.0f) {
            return 10.0f;
          } else if (xw <= 10.f) {
            return 0.0f;
          } else if (xw <= 15.f) {
            return 10.0f;
          } else {
            return 0.0f;
          }
        }
    };

    // Empty it.
    while (!points.empty()) {
     points.pop_back();
    }
    // How many segments (we make one fewer point).
    int segment_count = std::floor(params[SEGMENTS_PARAM].getValue());
    float distance = 10.0f / segment_count;
    // The scale of the square is 0,0 -> 10, 10.
    bool unipolar = getOffsetUnipolar();
    auto reset_func = reset_functions[reset_shape];
    float w = reset_type * 2.5f;
    if (startup && saveCurveInRack) {
      for (point p : loaded_points) {
        points.push_back(p);
      }
    } else {
      for (int i = 1; i < segment_count; i++) {
        point p = {i * distance, reset_func(i * distance, w, unipolar)};
        points.push_back(p);
      }
    }
    if (startup && saveCurveInRack) {
      start_point.y = loaded_start_y;
      end_point.y = loaded_end_y;
    } else {
      start_point.y = reset_func(0.0f, w, unipolar);
      end_point.y = reset_func(10.0f, w, unipolar);
    }
  }

  // Given two points that define a line, find y for the given x.
  float find_y(point point_0, point point_1, float x) {
    // Need to be careful if x1 == x0.
    float x_diff = std::max(point_1.x - point_0.x, 0.00001f);
    float a = (point_1.y - point_0.y) / x_diff;
    float b = point_0.y - a * point_0.x;
    return a * x + b;
  }

  // Given two points that define a line, find y for the given x.
  // Uses Smoothstep (https://en.wikipedia.org/wiki/Smoothstep) to smooth
  // out the lines.
  float find_smooth_y(point point_0, point point_1, float x) {
    // Need to be careful if x1 == x0.
    float x_diff = std::max(point_1.x - point_0.x, 0.00001f);
    float a = (point_1.y - point_0.y) / x_diff;
    float b = point_0.y - a * point_0.x;
    float raw_y = a * x + b;


    // Scale so that the lower y value is zero and the higher y value is one.
    // Remember the scaling factor, so you can transform back.
    float low_y = std::min(point_0.y, point_1.y);
    float high_y = std::max(point_0.y, point_1.y);
    if (std::abs(high_y - low_y) < 0.001f) {
      // Avoid divide by zero error.
      return raw_y;
    }
    float cooked_y = (raw_y - low_y) * (1.0f / (high_y - low_y));
    float smooth_y = (3.0f * cooked_y * cooked_y) -
      (2.0f * cooked_y * cooked_y * cooked_y);
    // Return to original coordinate space.
    return smooth_y * (high_y - low_y) + low_y;
  }

  // Given the two drift scales, current point, and a min and max point that
  // bounds the region, return a point in that region that is no more than
  // 'total drift' away from the current point and whose delta along the x_axis
  // is no more than 'x drift'.
  point uniform_region_value(float total_drift, float x_drift, point current,
                             point min, point max) {
    // We run this by creating points and seeing if they are close enough.
    // See https://www.youtube.com/watch?v=4y_nmpv-9lI for why.
    float x_range = std::min(x_drift, std::min(total_drift, max.x - min.x));
    float y_range = std::min(total_drift, max.y - min.y);
    point result;
    while (true) {
      float x_diff = random::uniform() * x_range - (x_range / 2.0f);
      float y_diff = random::uniform() * y_range - (y_range / 2.0f);
      // Test with Pythagorean theorem.
      if ((x_diff * x_diff) + (y_diff * y_diff) <= (total_drift * total_drift)) {
        result.x = current.x + x_diff;
        result.y = current.y + y_diff;
        // And test the boundaries.
        if ((min.x <= result.x) && (result.x <= max.x) &&
            (min.y <= result.y) && (result.y <= max.y)) {
          return result;
        }
      }
    }
  }

  void drift_point(float total_drift, float x_drift, unsigned int i) {
    float low_x, high_x;
    if (i == 0) {
      low_x = 0.0f;
    } else {
      // I think this has some bias to it, since the previous point will
      // have already moved.
      low_x = points[i - 1].x;
    }
    if (i == points.size() - 1) {
      high_x = 10.0f;
    } else {
      high_x = points[i + 1].x;
    }

    point this_point = points[i];
    point min, max;
    min.x = low_x + 0.001f;
    min.y = 0.0f;
    max.x = high_x - 0.001f;
    max.y = 10.0f;
    points[i] = uniform_region_value(total_drift, x_drift, this_point, min, max);
  }

  float compute_y_for_x(float domain, LineType line_type) {
    // Figure out which segment we are in.
    point prev = start_point, start, end;
    bool found_end = false;
    // For performance sake, make this a binary search?
    // Seems not, didn't look faster.

    // FYI: How much CPU is saved if I don't actually send output?
    // Surprisingly, it only goes from 0.7-8% -> 0.4-5%.
    for (std::vector<point>::iterator it = points.begin();
         it != points.end(); ++it) {
      if (domain < it->x) {
        start = prev;
        end = *it;
        found_end = true;
        break;
      } else {
        prev = *it;
      }
    }
    if (!found_end) {
      start = prev;
      end = end_point;
    }

    switch (line_type) {
    case LINES_LINETYPE:
      // Straight lines.
      return find_y(start, end, domain);
    case SMOOTHSTEP_LINETYPE:
      // Smoothed version of LINES.
      return find_smooth_y(start, end, domain);
    case STEPS_LINETYPE:
      // Steps.
      return start.y;
    default:
      // Something broken, should not happen!
      return 0.0f;
    }
  }

  void compute_display_points() {
    // Fill in display_points with the desired number of points to draw in the
    // display. We do this separately from the actual drawing, since this is
    // expensive and only needs to happen when the funtion actually changes.
    // We fill in in unipolar mode; the draw part can shift as needed.
    int type_knob = params[LINETYPE_PARAM].getValue();
    LineType line_type = LINES[type_knob];
    for (int i = 0; i < DISPLAY_POINT_COUNT; i++) {
      // "- 1" because need both ends.
      float x = i * (10.0f / (DISPLAY_POINT_COUNT - 1));
      float y = compute_y_for_x(x, line_type);
      display_points[i].x = x;
      display_points[i].y = y;
    }
  }

  bool getOffsetUnipolar() {
    return params[OFFSET_PARAM].getValue() > 0;
  }

  float getDomain() {
    float domain = inputs[DOMAIN_INPUT].getVoltage();
    if (!getOffsetUnipolar()) {
      domain += 5.0f;
    }
    return clamp(domain, 0.0f, 10.0f);
  }

  void process(const ProcessArgs& args) override {
    // Many events make us want to recompute the graph, but let's
    // only do that once per step.
    bool need_to_update_graph = false;
    // I'd have preferred to initialize 'points' in the constructor,
    // but it seems that the values of knobs (i.e., params) aren't
    // available in the constructor. So I wait until here, when they
    // are definitely set.
    if (!initialized) {
      reset_points(true);
      need_to_update_graph = true;  // Yes, all y's will be zero.
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

    // TODO: PERF: consider reading buttons only every N samples.

    // Y values of end points depend on the offset.
    bool offset_unipolar = getOffsetUnipolar();
    // TODO: start and end values should be set on reset, no other time.

    int type_knob = params[LINETYPE_PARAM].getValue();
    LineType line_type = LINES[type_knob];
    if (line_type != prev_line_type) {
      need_to_update_graph = true;
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

    if (reset) {
      reset_points(false);
      // Now that new function has been computed, update the display curve.
      need_to_update_graph = true;  // Yes, all y's will be zero.
    }

    // Determine if we have a DRIFT event from button or input.
    bool drift_was_low = !driftTrigger.isHigh();
    driftTrigger.process(rescale(
        inputs[DRIFT_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
    bool drift_from_input = drift_was_low && driftTrigger.isHigh();

    // We only want one drift from a button press.
    bool drift_from_button = false;
    if (params[DRIFT_PARAM].getValue() > 0.1f) {
      if (!drift_button_pressed) {
        drift_from_button = true;
        drift_button_pressed = true;
      }
    } else {
      drift_button_pressed = false;
    }

    // Actually drift, if asked to.
    if (!reset && (drift_from_input || drift_from_button)) {
      // Flash the drift light for a tenth of second.
      // Compute how many samples to show the light.
      // Note that, in contrast to RESET, we do set a timer on the drift
      // light; because we want to convey that DRIFT does ONE drift per
      // press, but RESET is just as reset no matter how long you hold it.
      drifting_light_countdown = std::floor(args.sampleRate / 10.0f);

      float x_drift = params[X_SCALE_PARAM].getValue();
      if (inputs[X_SCALE_INPUT].isConnected()) {
        // Don't allow x_drift to be negative.
        x_drift = clamp(x_drift + inputs[X_SCALE_INPUT].getVoltage(), 0.0f, 10.0f);
      }

      float total_drift = params[SCALE_PARAM].getValue();
      if (inputs[SCALE_INPUT].isConnected()) {
        // Don't allow total_drift to be negative.
        total_drift = clamp(total_drift + inputs[SCALE_INPUT].getVoltage(), 0.0f, 10.0f);
      }

      // To avoid the bias of always calculating limits from left -> right,
      // we alternate direction.
      left_to_right = !left_to_right;

      // Randomize locations of each point.
      for (unsigned int i = 0; i < points.size(); i++) {
        if (left_to_right) {
          drift_point(total_drift, x_drift, i);
        } else {
          drift_point(total_drift, x_drift, points.size() - 1 - i);
        }
      }
      if (endpoints_drift) {
        point min, max;
        min = {0.0f, 0.0f};
        max = {0.0f, 10.0f};
        point result = uniform_region_value(total_drift, 0.0f, start_point, min, max);
        start_point.y = result.y;

        min = {10.0f, 0.0f};
        max = {10.0f, 10.0f};
        result = uniform_region_value(total_drift, 0.0f, end_point, min, max);
        end_point.y = result.y;
      }

      // Now that new function has been computed, update the display curve.
      need_to_update_graph = true;  // Yes, all y's will be zero.
    }

    if (inputs[DOMAIN_INPUT].isConnected() &&
        outputs[RANGE_OUTPUT].isConnected()) {
      // Only need to compute the output if input and output are connected!
      float domain = getDomain();
      float range = compute_y_for_x(domain, line_type);
      if (!offset_unipolar) {
        range -= 5.0f;
      }
      outputs[RANGE_OUTPUT].setVoltage(range);
    }

    // All the reasons we might need to recompute the display graph.
    if (need_to_update_graph) {
      compute_display_points();
    }

    // Lights.
    lights[OFFSET_LIGHT].setBrightness(offset_unipolar);
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
  int reset_shape = 0;
  int reset_type = 0;

  // Loaded from the JSON; placed into points during reset_points.
  std::vector<point> loaded_points;
  float loaded_start_y, loaded_end_y;
};

struct DrifterDisplay : LedDisplay {
  Drifter* module;
  // We just use this to get the scope colors.
  ModuleWidget* moduleWidget;
  std::string fontPath;

  DrifterDisplay() {
    fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
  }

  // Transform from 0.0f -> 10.f to display point in nvg.
  Vec transform(point p, Vec bounding_box) {
    // "point" is in 0.0 -> 10.0 range.
    // Display has 0.0 in upper left corner.
    return Vec(p.x * bounding_box.x * 0.1f,
               (10.0f - p.y) * bounding_box.y * 0.1f);
  }

  void drawLayer(const DrawArgs& args, int layer) override {
    if ((layer == 1) && (module) && (moduleWidget) && module->initialized) {
      Rect r = box.zeroPos(); // .shrink(Vec(4, 5));  // TODO: ???
      Vec bounding_box = r.getBottomRight();
      Vec p0 = transform(module->display_points[0], bounding_box);

      // Draw middle line.
      nvgBeginPath(args.vg);
      nvgMoveTo(args.vg, 0.0, bounding_box.y / 2.0);
      nvgLineTo(args.vg, bounding_box.x, bounding_box.y / 2.0);
      nvgLineCap(args.vg, NVG_ROUND);
      nvgMiterLimit(args.vg, 2.f);
      nvgStrokeWidth(args.vg, 1.5f);
      nvgStrokeColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x80));
      nvgStroke(args.vg);

      // Draw voltage numbers, to make the IN and OUT range more obvious.
      std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
      if (font) {
        nvgFontSize(args.vg, 13);
        nvgFontFaceId(args.vg, font->handle);
        nvgTextLetterSpacing(args.vg, -2);

        std::string text = module->getOffsetUnipolar() ? "5" : "0";
        // Place on the line just off the left edge.
        nvgText(args.vg, 1, bounding_box.y / 2.0 + 4, text.c_str(), NULL);

        text = module->getOffsetUnipolar() ? "0" : "-5";
        // Place a little above the bottom just off the left edge.
        nvgText(args.vg, 1, bounding_box.y - 5, text.c_str(), NULL);

        text = module->getOffsetUnipolar() ? "10" : "5";
        // Place a little above the bottom just off the right edge.
        nvgText(args.vg, bounding_box.x - 12, bounding_box.y - 5, text.c_str(), NULL);
        // Place a little below the top just off the left edge.
        nvgText(args.vg, 1, 12, text.c_str(), NULL);
      }

      // Get line color from the OUT cable color, or white if not connected.
      PortWidget* output = moduleWidget->getOutput(Drifter::RANGE_OUTPUT);
      CableWidget* outputCable = APP->scene->rack->getTopCable(output);
      NVGcolor outputColor = outputCable ? outputCable->color : color::WHITE;

      // The graph of the output function.
      nvgBeginPath(args.vg);
      nvgMoveTo(args.vg, p0.x, p0.y);
      for (int i = 1; i < DISPLAY_POINT_COUNT; i++) {
        Vec next = transform(module->display_points[i], bounding_box);
        nvgLineTo(args.vg, next.x, next.y);
      }
      nvgLineCap(args.vg, NVG_ROUND);
      nvgMiterLimit(args.vg, 2.f);
      nvgStrokeWidth(args.vg, 1.5f);
      nvgStrokeColor(args.vg, outputColor);
      nvgStroke(args.vg);

      // And a short vertical line indicating the position of IN.
      if (module->inputs[Drifter::DOMAIN_INPUT].isConnected()) {
        // Get line color from the IN cable color.
        PortWidget* input = moduleWidget->getInput(Drifter::DOMAIN_INPUT);
        CableWidget* inputCable = APP->scene->rack->getTopCable(input);
        NVGcolor inputColor = inputCable ? inputCable->color : color::WHITE;
        float x = module->getDomain() * bounding_box.x * 0.1f;
        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, x, bounding_box.y);
        nvgLineTo(args.vg, x, bounding_box.y * 0.8);
        nvgLineCap(args.vg, NVG_ROUND);
        nvgMiterLimit(args.vg, 2.f);
        nvgStrokeWidth(args.vg, 1.5f);
        nvgStrokeColor(args.vg, inputColor);
        nvgStroke(args.vg);
      }
    }
    LedDisplay::drawLayer(args, layer);
  }
};

struct DrifterWidget : ModuleWidget {
  DrifterWidget(Drifter* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Drifter.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(
        Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(
        Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(
        Vec(box.size.x - 2 * RACK_GRID_WIDTH,
            RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    DrifterDisplay* display = createWidget<DrifterDisplay>(
        mm2px(Vec(0.360, 11.844)));
    display->box.size = mm2px(Vec(45.0, 30.0));
    display->module = module;
    display->moduleWidget = this;
    addChild(display);

    // OFST
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(37.224, 48.0)),
                                             module, Drifter::OFFSET_PARAM,
                                             Drifter::OFFSET_LIGHT));
    // ENDS
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(37.224, 64.0)),
                                             module, Drifter::ENDPOINTS_PARAM,
                                             Drifter::ENDPOINTS_LIGHT));

    // Line Count.
    addParam(createParamCentered<RoundBlackKnob>(
         mm2px(Vec(37.224, 80.0)), module, Drifter::SEGMENTS_PARAM));
    // Line Style
    RoundBlackSnapKnob* line_knob = createParamCentered<RoundBlackSnapKnob>(
         mm2px(Vec(37.224, 96.0)), module, Drifter::LINETYPE_PARAM);
    line_knob->minAngle = -0.28f * M_PI;
    line_knob->maxAngle = 0.28f * M_PI;
    addParam(line_knob);

    // X Drift.
    // Input.
    addInput(createInputCentered<PJ301MPort>(
        mm2px(Vec(8.024, 48.0)), module, Drifter::X_SCALE_INPUT));
    // Knob.
    addParam(createParamCentered<RoundBlackKnob>(
         mm2px(Vec(22.624, 48.0)), module, Drifter::X_SCALE_PARAM));

    // Total Drift.
    // Input.
    addInput(createInputCentered<PJ301MPort>(
        mm2px(Vec(8.024, 64.0)), module, Drifter::SCALE_INPUT));
    // Knob.
    addParam(createParamCentered<RoundBlackKnob>(
         mm2px(Vec(22.624, 64.0)), module, Drifter::SCALE_PARAM));

    // Commands from user/system.
    // Drift
    addInput(createInputCentered<PJ301MPort>(
        mm2px(Vec(8.024, 80.0)), module, Drifter::DRIFT_INPUT));
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(22.624, 80.0)),
                                             module, Drifter::DRIFT_PARAM,
                                             Drifter::DRIFT_LIGHT));

    // Reset
    addInput(createInputCentered<PJ301MPort>(
        mm2px(Vec(8.024, 96.0)), module, Drifter::RESET_INPUT));
    // Making this a Button and not a Latch means that it pops back up
    // when you let go.
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(22.624, 96.0)),
                                             module, Drifter::RESET_PARAM,
                                             Drifter::RESET_LIGHT));

    // Input
    addInput(createInputCentered<PJ301MPort>(
        mm2px(Vec(13.905, 112.000)), module, Drifter::DOMAIN_INPUT));

    // The Output
    addOutput(createOutputCentered<PJ301MPort>(
        mm2px(Vec(27.797, 112.000)), module, Drifter::RANGE_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    Drifter* module = dynamic_cast<Drifter*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolPtrMenuItem("Save curve in rack", "",
                                          &module->saveCurveInRack));
    menu->addChild(createMenuLabel("RESET shape:"));
    std::pair<std::string, int> shapes[] = {
      {"Horizontal line at zero", 0},
      {"Sine", 1},
      {"Triangle", 2},
      {"Rising Saw", 3},
      {"Falling Saw", 4},
      {"Square", 5},
    };
    std::pair<std::string, int> common_types[] = {
      {"A", 0},
      {"B", 1},
      {"C", 2},
      {"D", 3},
    };

    for (auto shape : shapes) {
      if (shape.second < 1) {
        menu->addChild(createCheckMenuItem(shape.first, "",
            [=]() {return shape.second == module->reset_shape;},
            [=]() {module->reset_shape = shape.second;}
        ));
      } else {
        int shape_num = shape.second;
        MenuItem* shape_menu = createSubmenuItem(shape.first, "",
          [=](Menu* menu) {
              for (auto common : common_types) {
                menu->addChild(createCheckMenuItem(common.first, "",
                  [=]() {return shape_num == module->reset_shape &&
                                module->reset_type == common.second;},
                  [=]() {module->reset_shape = shape_num;
                         module->reset_type = common.second;}
                ));
              }
          }
        );
        menu->addChild(shape_menu);
      }
    }
  }
};

Model* modelDrifter = createModel<Drifter, DrifterWidget>("Drifter");
