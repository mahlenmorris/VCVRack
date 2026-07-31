#include <algorithm>
#include <map>
#include <random>
#include <set>
#include <vector>

// #define MEASURE_FRAME_TIME 1
#ifdef MEASURE_FRAME_TIME
#include <chrono>
#endif

#include "plugin.hpp"

struct Chances : Module {
  static constexpr int PAIR_COUNT = 10;

  enum ParamId {
    CONTINUOUS_BUTTON_PARAM,
    STYLE_PARAM,

    ENUMS(VALUE_PARAM, PAIR_COUNT),
    ENUMS(COUNT_PARAM, PAIR_COUNT),
    PARAMS_LEN
  };
  enum InputId { TRIG_INPUT, POSITION_INPUT, INPUTS_LEN };
  enum OutputId { OUT_OUTPUT, OUTPUTS_LEN };
  enum LightId { CONTINUOUS_BUTTON_LIGHT, LIGHTS_LEN };

  // For detecting output triggers (polyphonic).
  dsp::SchmittTrigger inputTrigger[PORT_MAX_CHANNELS];
  float prev_values[PAIR_COUNT];
  int prev_counts[PAIR_COUNT];
  // A vector of possibilities. While doing this prohibits non-integral
  // COUNT_PARAM values, it the data structure we want anyway for Shuffling,
  // and makes for a simple Sampling as well.
  std::vector<float> samples;

  // Data for shuffling.
  std::mt19937 random_source;
  std::vector<float> shuffled_samples[PORT_MAX_CHANNELS];
  int shuffled_index[PORT_MAX_CHANNELS];

  // Data for no repeats.
  std::map<float, std::pair<int, int>> block_map;
  float last_output_value[PORT_MAX_CHANNELS] = {};

  // Data for input selection
  int input_range = 0;  // 0: [-5, 5], 1: [0, 10], 2: [-10, 10]

  Chances() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    // TODO: need to add a Trigger detector that means to reshuffle.
    configSwitch(STYLE_PARAM, 0, 3, 0, "Value Choosen by",
                 {"Sampling", "Shuffling", "No Repeats", "Input Selection"});
    // This has distinct values.
    getParamQuantity(STYLE_PARAM)->snapEnabled = true;

    for (int i = 0; i < PAIR_COUNT; ++i) {
      configParam(VALUE_PARAM + i, -10.f, 10.f, 0.f, "Value to possibly emit");
      configParam(COUNT_PARAM + i, 0, 100, 0,
                  "Count of relative chance that this value will be chosen");
      getParamQuantity(COUNT_PARAM + i)->snapEnabled = true;
    }

    // A latched button.
    configSwitch(CONTINUOUS_BUTTON_PARAM, 0, 1, 0,
                 "Ignore trigger and continuously output values.",
                 {"Off", "On"});

    configInput(TRIG_INPUT,
                "Triggers here will cause a new random number to "
                "be sent to the output.");
    configInput(POSITION_INPUT,
                "(Only for Input Selection STYLE.) Voltages here will select "
                "the appropriate output value.");
    configOutput(OUT_OUTPUT,
                 "Emits values according to the relative chances set above.");
    // Init with impossible values, to guarantee a refresh at the start.
    for (int pos = 0; pos < PAIR_COUNT; ++pos) {
      prev_values[pos] = -12.0;
      prev_counts[pos] = -1;
    }
    // Prepare a source of randomness that shuffle will like.
    // Seed with high-entropy source.
    std::random_device rd;

    // Initialize Mersenne Twister generator with seed.
    random_source.seed(rd());

    // Mark all channels as not having a shuffled_samples prepared yet.
    for (int c = 0; c < PORT_MAX_CHANNELS; ++c) {
      shuffled_index[c] = -1;
    }
  }

  // If asked to, save the curve data in json for reading when loaded.
  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "input_range", json_integer(input_range));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* input_range_json = json_object_get(rootJ, "input_range");
    if (input_range_json) {
      input_range = json_integer_value(input_range_json);
    }
  }

  void onRandomize(const RandomizeEvent& e) override {
    Module::onRandomize(e);

    // I'm fairly certain a user would want them sorted at this point.
    sortPairs();
  }

  void perform_shuffle(int channel) {
    // Copy the samples vector.
    shuffled_samples[channel].assign(samples.begin(), samples.end());

    // 3. Perform shuffle.
    std::shuffle(shuffled_samples[channel].begin(),
                 shuffled_samples[channel].end(), random_source);
    // Start from the end.
    shuffled_index[channel] = shuffled_samples[channel].size() - 1;
  }

  // Sorts the pairs so that:
  // The pairs with non-zero counts are "first", and those are sorted by
  // value.
  // Thus the pairs move left -> right the same as the vertical lines do.
  void sortPairs() {
    struct PairData {
      float value;
      float count;
    };
    std::vector<PairData> pairs;
    for (int i = 0; i < PAIR_COUNT; ++i) {
      pairs.push_back({params[VALUE_PARAM + i].getValue(),
                       params[COUNT_PARAM + i].getValue()});
    }

    std::sort(pairs.begin(), pairs.end(),
              [](const PairData& a, const PairData& b) {
                bool a_zero = (a.count <= 0.0f);
                bool b_zero = (b.count <= 0.0f);
                if (a_zero != b_zero) {
                  return b_zero;
                }
                return a.value < b.value;
              });

    for (int i = 0; i < PAIR_COUNT; ++i) {
      getParamQuantity(VALUE_PARAM + i)->setValue(pairs[i].value);
      getParamQuantity(COUNT_PARAM + i)->setValue(pairs[i].count);
    }
  }

  void process(const ProcessArgs& args) override {
    // Determine if we need to recompute the probability field.
    // If knobs have changed, then yes!
    bool need_update = false;

    for (int pos = 0; pos < PAIR_COUNT; ++pos) {
      if ((prev_values[pos] != params[VALUE_PARAM + pos].getValue()) ||
          (prev_counts[pos] != params[COUNT_PARAM + pos].getValue())) {
        need_update = true;
        break;
      }
    }
    if (need_update) {
      std::map<float, int> aggregated_counts;
      for (int pos = 0; pos < PAIR_COUNT; ++pos) {
        float value = params[VALUE_PARAM + pos].getValue();
        int count = params[COUNT_PARAM + pos].getValue();
        prev_values[pos] = value;
        prev_counts[pos] = count;
        if (count > 0) {
          aggregated_counts[value] += count;
        }
      }

      samples.clear();
      block_map.clear();
      for (const auto& pair : aggregated_counts) {
        block_map[pair.first] = {pair.second, samples.size()};
        for (int i = 0; i < pair.second; ++i) {
          samples.push_back(pair.first);
        }
      }
      // TODO: if shuffling, arrange for that? Or not until shuffle is
      // triggered? What if this is first entry?? Think more on what user
      // expects.
      // That decision might need to be a menu option.
      // TODO: this should dirty the Framebuffer?
    }

    // Determine if we need to shuffle.
    int style = std::round(params[STYLE_PARAM].getValue());
    if (style == 1) {
      // In case we are just switching to Shuffled, better shuffle now.
      for (int c = 0; c < PORT_MAX_CHANNELS; ++c) {
        if (shuffled_index[c] < 0) {
          perform_shuffle(c);
        }
      }
    }

    int channels = std::max(1, inputs[TRIG_INPUT].getChannels());
    // If we're using "Input Selection" STYLE, then the number of channels in
    // POSITION_INPUT is more significant.
    if (style == 3) {
      channels = std::max(1, inputs[POSITION_INPUT].getChannels());
    }
    outputs[OUT_OUTPUT].setChannels(channels);

    bool continuous = params[CONTINUOUS_BUTTON_PARAM].getValue() > 0.5f;
    for (int c = 0; c < channels; ++c) {
      // Time to output new value?
      bool trig_was_low = !inputTrigger[c].isHigh();
      inputTrigger[c].process(
          rescale(inputs[TRIG_INPUT].getVoltage(c), 0.1f, 2.f, 0.f, 1.f));
      bool trig_from_input = trig_was_low && inputTrigger[c].isHigh();

      if (trig_from_input || continuous) {
        if (style == 1) {
          // shuffling.
          if (shuffled_samples[c].size() > 0) {
            if (shuffled_index[c] < 0) {
              perform_shuffle(c);
            }
            float out_val = shuffled_samples[c].at(shuffled_index[c]);
            outputs[OUT_OUTPUT].setVoltage(out_val, c);
            last_output_value[c] = out_val;
            --shuffled_index[c];
          } else {
            outputs[OUT_OUTPUT].setVoltage(0.0f, c);
          }
        } else if (style == 2 && block_map.size() >= 2) {
          // no repeats.
          int n = 0;
          int s = 0;
          auto it = block_map.find(last_output_value[c]);
          if (it != block_map.end()) {
            n = it->second.first;
            s = it->second.second;
          }

          int valid_size = samples.size() - n;
          if (valid_size > 0) {
            size_t r = (size_t)floor(rack::random::uniform() * valid_size);
            size_t position = (r < (size_t)s) ? r : (r + n);
            float out_val = samples.at(position);
            outputs[OUT_OUTPUT].setVoltage(out_val, c);
            last_output_value[c] = out_val;
          } else {
            outputs[OUT_OUTPUT].setVoltage(0.0f, c);
          }
        } else if (style == 3) {
          // Input Selection
          if (samples.size() > 0) {
            float cv = inputs[POSITION_INPUT].getVoltage(c);
            float min_cv, max_cv;
            if (input_range == 1) {  // 0 to 10
              min_cv = 0.0f;
              max_cv = 10.0f;
            } else if (input_range == 2) {  // -10 to 10
              min_cv = -10.0f;
              max_cv = 10.0f;
            } else {  // -5 to 5
              min_cv = -5.0f;
              max_cv = 5.0f;
            }

            float normalized = (cv - min_cv) / (max_cv - min_cv);
            normalized = clamp(normalized, 0.0f, 1.0f);

            size_t index = (size_t)(normalized * samples.size());
            if (index >= samples.size()) index = samples.size() - 1;

            float out_val = samples.at(index);
            outputs[OUT_OUTPUT].setVoltage(out_val, c);
            last_output_value[c] = out_val;
          } else {
            outputs[OUT_OUTPUT].setVoltage(0.0f, c);
          }
        } else {
          // standard sampling.
          if (samples.size() > 0) {
            size_t position =
                (size_t)floor(rack::random::uniform() * samples.size());
            float out_val = samples.at(position);
            outputs[OUT_OUTPUT].setVoltage(out_val, c);
            last_output_value[c] = out_val;
          } else {
            outputs[OUT_OUTPUT].setVoltage(0.0f, c);
          }
        }
      }
    }

    // Lights.
    lights[CONTINUOUS_BUTTON_LIGHT].setBrightness(continuous ? 1.f : 0.f);
  }
};

struct ChancesDisplay : Widget {
  Chances* module;
  std::string fontPath;
  int hovered_pair = -1;

#ifdef MEASURE_FRAME_TIME
  std::chrono::nanoseconds total_time_nanos = std::chrono::nanoseconds(0);
  int frame_count = 0;
#endif

  ChancesDisplay() {
    fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
  }

  // TODO: We'll want a Framebuffer for this at some point?
  // As of July 29, 2026, the unbuffered drawLayer is consuming less than
  // 20ms every 40 seconds (when running at 30fps). I'll check this
  // every now and then, but that rate is likely not worth the complication
  // of adding a Framebuffer.

  // Which will require this to be draw().
  void drawLayer(const DrawArgs& args, int layer) override {
#ifdef MEASURE_FRAME_TIME
    auto start_time = std::chrono::high_resolution_clock::now();
#endif

    if (layer == 1) {
      float values[Chances::PAIR_COUNT] = {0};
      int counts[Chances::PAIR_COUNT] = {0};
      float current_outs[PORT_MAX_CHANNELS] = {0};
      int out_channels = 1;
      if (module) {
        // Get values from actual module.
        for (int i = 0; i < Chances::PAIR_COUNT; ++i) {
          values[i] = module->prev_values[i];
          counts[i] = module->prev_counts[i];
        }
        out_channels =
            std::max(1, module->outputs[Chances::OUT_OUTPUT].getChannels());
        for (int c = 0; c < out_channels; ++c) {
          current_outs[c] = module->outputs[Chances::OUT_OUTPUT].getVoltage(c);
        }
      } else {
        // Default values to show in module browser and library.
        values[0] = -2.5;
        counts[0] = 10;
        values[1] = 0.5;
        counts[1] = 16;
        values[2] = 1.2;
        counts[2] = 5;
        out_channels = 1;
        current_outs[0] = 0.5;
      }

      Rect r = box.zeroPos();  // .shrink(Vec(4, 5));  // TODO: ???
      Vec bounding_box = r.getBottomRight();

      nvgBeginPath(args.vg);
      nvgFillColor(args.vg, nvgRGBA(250, 250, 250, 255));

      std::map<float, int> aggregated_counts;
      int max_count = 0;
      for (int i = 0; i < Chances::PAIR_COUNT; ++i) {
        if (counts[i] > 0) {
          aggregated_counts[values[i]] += counts[i];
          if (aggregated_counts[values[i]] > max_count) {
            max_count = aggregated_counts[values[i]];
          }
        }
      }

      float rect_width = 3.0f;  // Skinnier rectangles

      float min_val = 0.0f;
      float max_val = 0.0f;
      float range = 0.0f;
      if (max_count > 0) {
        min_val = aggregated_counts.begin()->first;
        max_val = aggregated_counts.rbegin()->first;
        range = max_val - min_val;
      }

      // We should display at least one voltage indicator.
      range = std::max(range, 1.2f);

      // Make sure background lines and labels are not drawn outside the
      // display.
      nvgScissor(args.vg, RECT_ARGS(r));

      int y_interval = 1;
      if (max_count >= 500)
        y_interval = 100;
      else if (max_count >= 200)
        y_interval = 50;
      else if (max_count >= 80)
        y_interval = 25;
      else if (max_count >= 35)
        y_interval = 10;
      else if (max_count >= 16)
        y_interval = 5;
      else if (max_count >= 8)
        y_interval = 2;

      // Draw faint grid background
      nvgBeginPath(args.vg);
      // Horizontal lines (Y-axis / counts)
      if (max_count > 0) {
        for (int c = y_interval; c <= max_count; c += y_interval) {
          float y = bounding_box.y -
                    (static_cast<float>(c) / max_count) * bounding_box.y;
          nvgMoveTo(args.vg, 0, y);
          nvgLineTo(args.vg, bounding_box.x, y);
        }
      }

      // Vertical lines (X-axis / voltages)
      if (max_count > 0) {
        if (range > 0.0f) {
          int start_v = std::floor(min_val);
          int end_v = std::ceil(max_val);
          for (int v = start_v; v <= end_v; ++v) {
            float drawable_width = bounding_box.x - rect_width;
            float mapped_x =
                (rect_width / 2.0f) + ((v - min_val) / range) * drawable_width;
            nvgMoveTo(args.vg, mapped_x, 0);
            nvgLineTo(args.vg, mapped_x, bounding_box.y);
          }
        } else {
          // If all values are the same, draw one vertical line in the center
          nvgMoveTo(args.vg, bounding_box.x / 2.0f, 0);
          nvgLineTo(args.vg, bounding_box.x / 2.0f, bounding_box.y);
        }
      }

      nvgStrokeColor(args.vg, nvgRGBA(255, 255, 255, 30));
      nvgStrokeWidth(args.vg, 1.0f);
      nvgStroke(args.vg);

      // Draw rectangles for PDF
      if (max_count > 0) {
        for (const auto& pair : aggregated_counts) {
          float val = pair.first;
          int count = pair.second;

          float mapped_x;
          if (range > 0.0f) {
            // Map value from [min_val, max_val] to fit within the box width
            // We subtract rect_width from bounding box so the edges don't clip.
            float drawable_width = bounding_box.x - rect_width;
            mapped_x = (rect_width / 2.0f) +
                       ((val - min_val) / range) * drawable_width;
          } else {
            // If all values are the same, draw in the center
            mapped_x = bounding_box.x / 2.0f;
          }

          // Center the rectangle on the mapped X coordinate
          float x = mapped_x - (rect_width / 2.0f);

          float height =
              (static_cast<float>(count) / max_count) * bounding_box.y;
          float y = bounding_box.y - height;

          nvgBeginPath(args.vg);
          if (hovered_pair != -1 && val == values[hovered_pair]) {
            nvgFillColor(args.vg, nvgRGBA(0, 255, 255, 255));  // Cyan highlight
          } else {
            nvgFillColor(args.vg, nvgRGBA(250, 250, 250, 255));  // Normal white
          }
          nvgRect(args.vg, x, y, rect_width, height);
          nvgFill(args.vg);
        }

        // If a knob is hovered, draw a faint vertical cyan line so the user can
        // see where it points even if the count is zero.
        if (hovered_pair != -1 && range > 0.0f) {
          float h_val = values[hovered_pair];
          // Only draw the line if it falls within our current min/max display
          // range
          if (h_val >= min_val && h_val <= max_val) {
            float drawable_width = bounding_box.x - rect_width;
            float h_mapped_x = (rect_width / 2.0f) +
                               ((h_val - min_val) / range) * drawable_width;
            nvgBeginPath(args.vg);
            nvgMoveTo(args.vg, h_mapped_x, 0);
            nvgLineTo(args.vg, h_mapped_x, bounding_box.y);
            nvgStrokeColor(args.vg, nvgRGBA(0, 255, 255, 100));
            nvgStrokeWidth(args.vg, 1.0f);
            nvgStroke(args.vg);
          }
        }

        // Draw current output indicators (red triangles).
        // Collect unique active output values.
        std::set<float> active_outs;
        for (int c = 0; c < out_channels; ++c) {
          float current_out = current_outs[c];
          bool matches = false;
          for (int i = 0; i < Chances::PAIR_COUNT; ++i) {
            if (counts[i] > 0 && std::abs(values[i] - current_out) < 1e-4f) {
              matches = true;
              break;
            }
          }
          if (matches) {
            active_outs.insert(current_out);
          }
        }

        for (float current_out : active_outs) {
          float out_mapped_x;
          if (range > 0.0f) {
            float drawable_width = bounding_box.x - rect_width;
            out_mapped_x = (rect_width / 2.0f) +
                           ((current_out - min_val) / range) * drawable_width;
          } else {
            out_mapped_x = bounding_box.x / 2.0f;
          }

          // Draw a small red triangle pointing up from the bottom edge
          nvgBeginPath(args.vg);
          nvgMoveTo(args.vg, out_mapped_x, bounding_box.y - 6.0f);
          nvgLineTo(args.vg, out_mapped_x - 3.5f, bounding_box.y);
          nvgLineTo(args.vg, out_mapped_x + 3.5f, bounding_box.y);
          nvgClosePath(args.vg);
          nvgFillColor(args.vg, SCHEME_RED);
          nvgFill(args.vg);
        }
      }

      // Draw text labels for integer voltages.
      std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
      if (font && max_count > 0 && range > 0.0f) {
        nvgSave(args.vg);
        nvgFontSize(args.vg, 11);
        nvgFontFaceId(args.vg, font->handle);
        nvgTextLetterSpacing(args.vg, -1);
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);

        // NanoVG does not have a built-in NVG_DIFFERENCE enum, but we can
        // easily synthesize it! By using custom blend functions and multiplying
        // the source (white text) by (1 - destination color), it perfectly
        // inverts whatever is underneath the text.
        nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR,
                                    NVG_ONE_MINUS_SRC_COLOR);
        nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 255));

        int start_v = std::floor(min_val);
        int end_v = std::ceil(max_val);
        for (int v = start_v; v <= end_v; ++v) {
          float drawable_width = bounding_box.x - rect_width;
          float mapped_x =
              (rect_width / 2.0f) + ((v - min_val) / range) * drawable_width;
          nvgText(args.vg, mapped_x, 2, std::to_string(v).c_str(), NULL);
        }

        // Text labels for counts (Y-axis)
        nvgTextAlign(args.vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
        for (int c = y_interval; c <= max_count; c += y_interval) {
          float y = bounding_box.y -
                    (static_cast<float>(c) / max_count) * bounding_box.y;
          // Slight padding from the right edge, drawn vertically centered on
          // the line
          nvgText(args.vg, bounding_box.x - 2, y, std::to_string(c).c_str(),
                  NULL);
        }
        nvgRestore(args.vg);
      }
      nvgResetScissor(args.vg);
    }

#ifdef MEASURE_FRAME_TIME
    if (layer == 1) {
      auto elapsed = std::chrono::high_resolution_clock::now() - start_time;
      total_time_nanos = total_time_nanos + elapsed;
      ++frame_count;
      if (frame_count >= 1000) {
        double average_frame_time = total_time_nanos.count() /
                                    (double)frame_count /
                                    1000.0;  // Convert to microseconds.
        WARN("Average frame time after %d frames: %f us", frame_count,
             average_frame_time);
        frame_count = 0;
        total_time_nanos = std::chrono::nanoseconds(0);
      }
    }
#endif
  }
};

struct ChancesKnob : RoundBlackKnob {
  ChancesDisplay* display = nullptr;
  int pair_index = -1;

  void onEnter(const EnterEvent& e) override {
    RoundBlackKnob::onEnter(e);
    if (display) display->hovered_pair = pair_index;
  }

  void onLeave(const LeaveEvent& e) override {
    RoundBlackKnob::onLeave(e);
    if (display && display->hovered_pair == pair_index) {
      display->hovered_pair = -1;
    }
  }
};

struct ChancesWidget : ModuleWidget {
  static constexpr float X_DIFF_MM = 11.5;

  ChancesWidget(Chances* module) {
    setModule(module);
    setPanel(
        // TODO: add dark version.
        createPanel(asset::plugin(pluginInstance, "res/Chances.svg")));

    ChancesDisplay* display =
        createWidget<ChancesDisplay>(mm2px(Vec(1.9, 11.5)));
    display->box.size = mm2px(Vec(72.0, 29.0));
    display->module = module;
    addChild(display);

    // The value-count pairs.
    for (int pos = 0; pos < 5; ++pos) {
      ChancesKnob* ck_count = createParamCentered<ChancesKnob>(
          mm2px(Vec(22.0 + pos * X_DIFF_MM, 46.0)), module,
          Chances::COUNT_PARAM + pos);
      ck_count->display = display;
      ck_count->pair_index = pos;
      addParam(ck_count);

      ChancesKnob* ck_val = createParamCentered<ChancesKnob>(
          mm2px(Vec(22.0 + pos * X_DIFF_MM, 56.0)), module,
          Chances::VALUE_PARAM + pos);
      ck_val->display = display;
      ck_val->pair_index = pos;
      addParam(ck_val);
    }
    for (int pos = 5; pos < Chances::PAIR_COUNT; ++pos) {
      ChancesKnob* ck_count = createParamCentered<ChancesKnob>(
          mm2px(Vec(22.0 + (pos - 5) * X_DIFF_MM, 69.0)), module,
          Chances::COUNT_PARAM + pos);
      ck_count->display = display;
      ck_count->pair_index = pos;
      addParam(ck_count);

      ChancesKnob* ck_val = createParamCentered<ChancesKnob>(
          mm2px(Vec(22.0 + (pos - 5) * X_DIFF_MM, 79.0)), module,
          Chances::VALUE_PARAM + pos);
      ck_val->display = display;
      ck_val->pair_index = pos;
      addParam(ck_val);
    }

    addParam(
        createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(
            mm2px(Vec(21.822, 116.0)), module, Chances::CONTINUOUS_BUTTON_PARAM,
            Chances::CONTINUOUS_BUTTON_LIGHT));

    RoundBlackKnob* style_knob = createParamCentered<RoundBlackKnob>(
        mm2px(Vec(38.0, 116.0)), module, Chances::STYLE_PARAM);
    style_knob->minAngle = -0.28f * M_PI;
    style_knob->maxAngle = 0.28f * M_PI;
    addParam(style_knob);

    addInput(createInputCentered<ThemedPJ301MPort>(
        mm2px(Vec(8.032, 116.0)), module, Chances::TRIG_INPUT));

    addInput(createInputCentered<ThemedPJ301MPort>(
        mm2px(Vec(53.0, 116.0)), module, Chances::POSITION_INPUT));

    addOutput(createOutputCentered<ThemedPJ301MPort>(
        mm2px(Vec(68.819, 116.0)), module, Chances::OUT_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    Chances* module = dynamic_cast<Chances*>(this->module);
    if (!module) return;

    menu->addChild(new MenuSeparator);

    // Option to sort the pairs.
    menu->addChild(createMenuItem("Sort pairs by value now", "",
                                  [=]() { module->sortPairs(); }));

    std::pair<std::string, int> input_ranges[] = {
        {"[-5V, 5V]", 0}, {"[0V, 10V]", 1}, {"[-10V, 10V]", 2}};

    MenuItem* range_menu =
        createSubmenuItem("Input Selection Range", "", [=](Menu* menu) {
          for (auto line : input_ranges) {
            menu->addChild(createCheckMenuItem(
                line.first, "",
                [=]() { return line.second == module->input_range; },
                [=]() { module->input_range = line.second; }));
          }
        });
    menu->addChild(range_menu);
  }
};

Model* modelChances = createModel<Chances, ChancesWidget>("Chances");
