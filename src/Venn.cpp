#include "plugin.hpp"

// The fundamental object being drawn and interacted with.
struct Circle {
  float x_center, y_center;  // Values from [0, 10).
  // 0, 0 is lower left corner; 10, 10 is upper right.
  float radius;
  std::string name;
};

struct Venn : Module {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    X_POSITION_INPUT,
    Y_POSITION_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    DISTANCE_OUTPUT,
    WITHIN_GATE_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Venn() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(X_POSITION_INPUT, "");
    configInput(Y_POSITION_INPUT, "");
    configOutput(DISTANCE_OUTPUT, "");
    configOutput(WITHIN_GATE_OUTPUT, "");

    // Just to have something to start with, here are two overlapping Circles.
    Circle first;
    first.x_center = 6.0;
    first.y_center = 5.0;
    first.radius = 3.5;
    circles.push_back(first);
    Circle second;
    second.x_center = 4.0;
    second.y_center = 5.5;
    second.radius = 3.0;
    circles.push_back(second);

    point.x = 5;
    point.y = 5;
  }

  // Save and retrieve menu choice(s).
  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    /*
    json_t* default_inJ = json_object_get(rootJ, "default_in");
    if (default_inJ) {
      default_in_voltage = json_real_value(default_inJ);
    }
    */
  }

  void process(const ProcessArgs& args) override {
    // Update X and Y inputs.
    point.x = inputs[X_POSITION_INPUT].getVoltage();
    point.y = inputs[Y_POSITION_INPUT].getVoltage();

    // Determine what values to output.
    // TODO: many optimizations, including doing nothing when neither point nor circles has changed.
    // TODO: Can SIMD library help me?

    // TODO: We will need deleted circles to make this work when we can delete circles.
    outputs[DISTANCE_OUTPUT].setChannels(circles.size());
    outputs[WITHIN_GATE_OUTPUT].setChannels(circles.size());

    for (size_t channel = 0; channel < circles.size(); channel++) {
      const Circle& circle = circles.at(channel);
      float x_distance = point.x - circle.x_center;
      float y_distance = point.y - circle.y_center;
      float distance = sqrt(x_distance * x_distance + y_distance * y_distance);

      if (distance > circle.radius) {
        outputs[DISTANCE_OUTPUT].setVoltage(0.0f, channel);
        outputs[WITHIN_GATE_OUTPUT].setVoltage(0.0f, channel);
      } else {
        // We'll do linear distance scaling for now.
        outputs[DISTANCE_OUTPUT].setVoltage((1 - distance / circle.radius) * 10, channel);
        outputs[WITHIN_GATE_OUTPUT].setVoltage(10.0f, channel);
      }
    }
  }

  std::vector<Circle> circles;
  Vec point;
};

struct CircleDisplay : Widget {
  Venn* module;

  CircleDisplay() {}


  double nvg_x(float volt_x, double size) { 
    return volt_x * size / 10.0;
  }

  double nvg_y(float volt_y, double size) { 
    // Need to invert Y, since I want to put y == 0 on the bottom.
    return size - (volt_y * size / 10.0);
  }
  
  // By using drawLayer() instead of draw(), this becomes a glowing Depict
  // when the "room lights" are turned down. That seems correct to me.
  void drawLayer(const DrawArgs& args, int layer) override {
    if (layer == 1) {
      Rect r = box.zeroPos();
      Vec bounding_box = r.getBottomRight();
      // Assuming that we are always on a square-pixeled surface, with X and Y the same distances.
      double pixels_per_volt = bounding_box.x / 10.0;

      // The circles.
      // TODO: add number, name, and vary the color (same scheme as Memory?).
      for (const Circle& circle : module->circles) {
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, nvg_x(circle.x_center, bounding_box.x), nvg_y(circle.y_center, bounding_box.x),
                pixels_per_volt * circle.radius);
        nvgStrokeColor(args.vg, SCHEME_BLUE);
        nvgStroke(args.vg);
      }

      // Draw the pointer (need a name for it).
      nvgBeginPath(args.vg);
      nvgCircle(args.vg, nvg_x(module->point.x, bounding_box.x), nvg_y(module->point.y, bounding_box.x),
              pixels_per_volt * 0.15);
      nvgStrokeColor(args.vg, SCHEME_BLACK);
      nvgStroke(args.vg);

    }
/*
    if (layer == 1) {
      int max_distance;
      int line_record_size;
      int buffer_length;
      PointBuffer* waveform;
      // True iff point_buffer was allocated just for this call.
      bool free_point_buffer = false;

      if (module) {
        max_distance = std::max(1, module->max_distance + 1);
        line_record_size = (int) module->line_records.size();
        std::shared_ptr<Buffer> buffer = module->buffer;  // In case it gets reset by another action.
        if (buffer && buffer->IsValid()) {
          waveform = &(buffer->waveform);
          buffer_length = buffer->length;
        } else {
          waveform = new PointBuffer();
          FillDummyWaveform(waveform, false);
          buffer_length = 100000;
          free_point_buffer = true;
        }
      } else {
        // Dummy data for the module browser.
        max_distance = 5;  // I'll have four heads.
        line_record_size = 4;
        waveform = new PointBuffer();
        FillDummyWaveform(waveform, true);
        free_point_buffer = true;
        buffer_length = 10;
      }
       
      // just in case max_distance is zero somehow, I don't want to divide by it.
      Rect r = box.zeroPos();
      Vec bounding_box = r.getBottomRight();

      double x_per_volt = (bounding_box.x - 1.0) / 20.0;
      double zero_volt_left = bounding_box.x / 2 - 0.5;
      double zero_volt_right = bounding_box.x / 2 + 0.5;
      double y_per_point = bounding_box.y / WAVEFORM_SIZE;

      // Draw the wave forms.
      // In one shape we:
      // * Draw the left side side from bottom to top.
      // * Draw the right side line from top to bottom.
      // * Join them. Draw that shape.
      // Then draw a white line down the middle to suggest that these are
      // two separate channels.

      // Make half-white.
      nvgFillColor(args.vg, nvgRGBA(140, 140, 140, 255));

      nvgSave(args.vg);
      nvgScissor(args.vg, RECT_ARGS(r));  // Not sure this is right?
      nvgBeginPath(args.vg);

      // Draw left points on the left of the mid.
      for (int i = 0; i < WAVEFORM_SIZE; i++) {
        float max = waveform->points[i][0] * waveform->normalize_factor;
        // We'll say the x position ranges from -10V to 10V.
        float x = zero_volt_left - (max * x_per_volt);
        float y = (WAVEFORM_SIZE - i) * y_per_point;
        if (i == 0) {
          nvgMoveTo(args.vg, x, y);
        } else {
          nvgLineTo(args.vg, x, y);
        }
      }

      // Now do the right channel.
      for (int i = WAVEFORM_SIZE - 1; i >= 0; i--) {
        float max = waveform->points[i][1] * waveform->normalize_factor;
        float x = zero_volt_right + (max * x_per_volt);
        float y = (WAVEFORM_SIZE - 1 - i) * y_per_point;
        nvgLineTo(args.vg, x, y);
      }

      nvgClosePath(args.vg);
      nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
      nvgFill(args.vg);

      // Draw line.
      nvgBeginPath(args.vg);
      nvgRect(args.vg, zero_volt_left, 0, 0.5f, bounding_box.y);
      nvgFillColor(args.vg, SCHEME_WHITE);
      nvgFill(args.vg);

      // Add text to indicate the largest value we currently display.
      nvgBeginPath(args.vg);
      nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
      nvgFillColor(args.vg, SCHEME_BLUE);
      nvgFontSize(args.vg, 11);
      // Do I need this? nvgFontFaceId(args.vg, font->handle);
      nvgTextLetterSpacing(args.vg, -1);

      // Place on the line just off the left edge.
      nvgText(args.vg, 4, 10, waveform->text_factor.c_str(), NULL);

      // Restore previous state.
      nvgResetScissor(args.vg);
      nvgRestore(args.vg);

      // Then draw the recording heads on top.
      for (int i = 0; i < line_record_size; i++) {
        LineRecord line;
        if (module) {
          line = module->line_records[i];
        } else {
          line = dummy_lines[i];
        }
        nvgBeginPath(args.vg);
        // I picture 0.0 at the bottom. TODO: is that a good idea?
        double y_pos = bounding_box.y *
                        (1 - ((double) line.position / buffer_length));
        // Line is changed by distance and type.
        if (line.type == RUMINATE) {
          // Endpoint of line suggests which module it is.
          double len = bounding_box.x * line.distance / max_distance;
          nvgRect(args.vg, 0.0, y_pos, len, 1);
        } else if (line.type == EMBELLISH) {
          double len = bounding_box.x * (max_distance - line.distance) / max_distance;
          nvgRect(args.vg, bounding_box.x - len, y_pos, len, 2);
        } else if (line.type == FIXATION) {
          double center = bounding_box.x * line.distance / max_distance;
          // Make these lines one fifth of the way across. Sure they'll overlap sometimes,
          // but they typically don't travel the whole vertical length of the buffer, and
          // they have to show up againt the white waveform.
          double len = bounding_box.x * 0.2;
          nvgRect(args.vg, center - len / 2.0, y_pos, len, 2);
        }
        nvgFillColor(args.vg, line.color);
        nvgFill(args.vg);
      }

      if (free_point_buffer) {
        delete waveform;
      }
    }
    */
	}
};


struct VennWidget : ModuleWidget {
  VennWidget(Venn* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Venn.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.024, 48.0)), module, Venn::X_POSITION_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.989, 47.993)), module, Venn::Y_POSITION_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(174.36, 12.435)), module, Venn::DISTANCE_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(174.625, 30.162)), module, Venn::WITHIN_GATE_OUTPUT));

    // The Circles.
    CircleDisplay* display = createWidget<CircleDisplay>(
      mm2px(Vec(26.19, 1.5)));
    display->box.size = mm2px(Vec(125.0, 125.0));
    display->module = module;
    addChild(display);

  }
};


Model* modelVenn = createModel<Venn, VennWidget>("Venn");