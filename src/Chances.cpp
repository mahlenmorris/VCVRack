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
    SPREAD_TYPE_PARAM,
    SPREAD_PARAM,
    SORT_PARAM,
    ENUMS(VALUE_PARAM, PAIR_COUNT),
    ENUMS(COUNT_PARAM, PAIR_COUNT),
    PARAMS_LEN
  };
  enum InputId { TRIG_INPUT, POSITION_INPUT, RESET_INPUT, INPUTS_LEN };
  enum OutputId { OUT_OUTPUT, OUTPUTS_LEN };
  enum LightId { CONTINUOUS_BUTTON_LIGHT, SORT_LIGHT, LIGHTS_LEN };

  // For detecting input triggers (polyphonic).
  dsp::SchmittTrigger inputTrigger[PORT_MAX_CHANNELS];
  dsp::SchmittTrigger resetTrigger[PORT_MAX_CHANNELS];
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

  // Data for displaying the red triangles.
  float last_output_value[PORT_MAX_CHANNELS] = {};

  // Data for input selection
  int input_range = 0;  // 0: [-5, 5], 1: [0, 10], 2: [-10, 10]

  // For sorting.
  dsp::SchmittTrigger sortTrigger;
  int sort_light_countdown = 0;
  bool sort_button_pressed = false;  // Only sort once per press.

  // Unless otherwise indicated, output this many channels.
  int default_out_channel_count = 1;

  Chances() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
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

    // A momentary button.
    configSwitch(SORT_PARAM, 0, 1, 0, "Press to sort pairs by value",
                 {"Off", "On"});

    // A 2-position toggle switch (values: 0, 1)
    configSwitch(SPREAD_TYPE_PARAM, 0.0f, 1.0f, 0.0f, "Kind of Spread",
                 {"Gaussian", "Uniform"});

    configParam(
        SPREAD_PARAM, 0.0f, 2.0f, 0.0f,
        "Allow outputs to randomly be at most this many volts away from "
        "their value");

    // A latched button.
    configSwitch(CONTINUOUS_BUTTON_PARAM, 0, 1, 0,
                 "Ignore trigger and continuously output values",
                 {"Off", "On"});

    configInput(TRIG_INPUT,
                "Triggers here will cause a new random number to "
                "be sent to the output");
    configInput(POSITION_INPUT,
                "(Only for Input Selection STYLE.) Voltages here will select "
                "the appropriate output value");
    configInput(RESET_INPUT,
                "Triggers here reset sequence state (forces reshuffle in "
                "Shuffling, clears repeat memory in No Repeats)");
    configOutput(OUT_OUTPUT,
                 "Emits values according to the relative chances set above");
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
      last_output_value[c] = -100.0f;
    }
  }

  // If asked to, save the curve data in json for reading when loaded.
  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "input_range", json_integer(input_range));
    json_object_set_new(rootJ, "default_out_channel_count",
                        json_integer(default_out_channel_count));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* input_range_json = json_object_get(rootJ, "input_range");
    if (input_range_json) {
      input_range = json_integer_value(input_range_json);
    }
    json_t* default_out_channel_count_json =
        json_object_get(rootJ, "default_out_channel_count");
    if (default_out_channel_count_json) {
      default_out_channel_count =
          json_integer_value(default_out_channel_count_json);
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
    // Some lights are lit by triggers or button presses; these enable them to
    // be lit long enough to be seen by humans.
    if (sort_light_countdown > 0) {
      sort_light_countdown--;
    }

    if ((params[SORT_PARAM].getValue() > 0.1f)) {
      if (!sort_button_pressed) {
        sort_button_pressed = true;
        // Do the sorting.
        sortPairs();
        // Light up for one tenth of a second.
        sort_light_countdown = std::floor(args.sampleRate / 10.0f);
      }
    } else {
      sort_button_pressed = false;
    }

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
      // Build samples and block_map at the same time.
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

    // Need to have distinct notions of input channels vs output channels.
    // This allows me to have a single clock signal to TRIG that creates N
    // outputs.
    int in_channels = std::max(1, inputs[TRIG_INPUT].getChannels());
    int out_channels =
        in_channels < 2 ? default_out_channel_count : in_channels;
    // If we're using "Input Selection" STYLE, then the number of channels in
    // POSITION_INPUT is more significant.
    if (style == 3) {
      out_channels = std::max(1, inputs[POSITION_INPUT].getChannels());
    }
    outputs[OUT_OUTPUT].setChannels(out_channels);

    bool continuous = params[CONTINUOUS_BUTTON_PARAM].getValue() > 0.5f;
    bool universal_trigger = false;
    // If we only have one channel on TRIG input and it triggers, that applies
    // to multiple output channels.
    if (inputs[TRIG_INPUT].getChannels() == 1) {
      bool trig_was_low = !inputTrigger[0].isHigh();
      inputTrigger[0].process(
          rescale(inputs[TRIG_INPUT].getVoltage(0), 0.1f, 2.f, 0.f, 1.f));
      universal_trigger = trig_was_low && inputTrigger[0].isHigh();
    }

    int reset_channels = inputs[RESET_INPUT].getChannels();
    bool universal_reset = false;
    if (reset_channels == 1) {
      bool reset_was_low = !resetTrigger[0].isHigh();
      resetTrigger[0].process(
          rescale(inputs[RESET_INPUT].getVoltage(0), 0.1f, 2.f, 0.f, 1.f));
      universal_reset = reset_was_low && resetTrigger[0].isHigh();
    }

    for (int curr_channel = 0; curr_channel < out_channels; ++curr_channel) {
      // Check for reset trigger
      bool reset_from_input = false;
      if (universal_reset) {
        reset_from_input = true;
      } else if (reset_channels > 1) {
        bool reset_was_low = !resetTrigger[curr_channel].isHigh();
        resetTrigger[curr_channel].process(rescale(
            inputs[RESET_INPUT].getVoltage(curr_channel), 0.1f, 2.f, 0.f, 1.f));
        reset_from_input = reset_was_low && resetTrigger[curr_channel].isHigh();
      }

      if (reset_from_input) {
        if (style == 1) {
          perform_shuffle(curr_channel);
        } else if (style == 2) {
          last_output_value[curr_channel] = -100.0f;
        }
      }

      // Time to output new value?
      bool trig_from_input = false;
      if (universal_trigger) {
        // Just one TRIG channel, it it triggered? Yes, we have a new output.
        trig_from_input = true;
      } else if (in_channels > 1) {
        // if there is more than in channel to TRIG, then we check them
        // individually.
        bool trig_was_low = !inputTrigger[curr_channel].isHigh();
        inputTrigger[curr_channel].process(rescale(
            inputs[TRIG_INPUT].getVoltage(curr_channel), 0.1f, 2.f, 0.f, 1.f));
        trig_from_input = trig_was_low && inputTrigger[curr_channel].isHigh();
      }

      if (trig_from_input || continuous) {
        float out_val = 0.0f;
        bool has_val = false;

        if (style == 1) {
          // shuffling.
          if (shuffled_samples[curr_channel].size() > 0) {
            if (shuffled_index[curr_channel] < 0) {
              perform_shuffle(curr_channel);
            }
            out_val =
                shuffled_samples[curr_channel].at(shuffled_index[curr_channel]);
            has_val = true;
            --shuffled_index[curr_channel];
          }
        } else if (style == 2 && block_map.size() >= 2) {
          // No Repeats STYLE.
          // In the span of samples, we block out a region of choices.
          // For example:
          // [n n n n n n n n n n n n n n n n]
          //        x x x x x x
          // Here. block_start is 3 and block_length = 6.
          int block_length = 0;
          int block_start = 0;
          auto it = block_map.find(last_output_value[curr_channel]);
          if (it != block_map.end()) {
            block_length = it->second.first;
            block_start = it->second.second;
          }

          int valid_size = samples.size() - block_length;
          if (valid_size > 0) {
            size_t r = (size_t)floor(rack::random::uniform() * valid_size);
            size_t position =
                (r < (size_t)block_start) ? r : (r + block_length);
            out_val = samples.at(position);
            has_val = true;
          }
        } else if (style == 3) {
          // Input Selection
          if (samples.size() > 0) {
            float range_min, range_max;
            if (input_range == 1) {  // 0 to 10
              range_min = 0.0f;
              range_max = 10.0f;
            } else if (input_range == 2) {  // -10 to 10
              range_min = -10.0f;
              range_max = 10.0f;
            } else {  // -5 to 5
              range_min = -5.0f;
              range_max = 5.0f;
            }

            size_t index = (size_t)(rescale(
                inputs[POSITION_INPUT].getVoltage(curr_channel), range_min,
                range_max, 0.0, samples.size()));
            // Let's just make really sure we never exceed the bounds.
            // After all, user could be wrong about the actual input_range.
            index = clamp(index, 0, samples.size() - 1);
            out_val = samples.at(index);
            has_val = true;
          }
        } else {
          // standard sampling.
          if (samples.size() > 0) {
            size_t position =
                (size_t)floor(rack::random::uniform() * samples.size());
            out_val = samples.at(position);
            has_val = true;
          }
        }

        if (has_val) {
          last_output_value[curr_channel] = out_val;
          float fuzziness = params[SPREAD_PARAM].getValue();
          float actual_out = out_val;
          // Add spread, if any.
          if (fuzziness > 0.0f) {
            float noise;
            if (params[SPREAD_TYPE_PARAM].getValue() == 0) {
              // An approximation of a Gaussian. Known as the Irwin-Hall
              // distribution curve, I recently discovered.
              noise = ((rack::random::uniform() + rack::random::uniform() +
                        rack::random::uniform()) /
                           3.0f -
                       0.5f) *
                      2.0f * fuzziness;
            } else {
              noise = rescale(rack::random::uniform(), 0.0, 1.0, -fuzziness,
                              fuzziness);
            }
            actual_out += noise;
          }
          outputs[OUT_OUTPUT].setVoltage(actual_out, curr_channel);
        } else {
          outputs[OUT_OUTPUT].setVoltage(0.0f, curr_channel);
        }
      }
    }

    // Lights.
    lights[CONTINUOUS_BUTTON_LIGHT].setBrightness(continuous ? 1.f : 0.f);
    lights[SORT_LIGHT].setBrightness(sort_light_countdown > 0 ? 1.f : 0.f);
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

  ChancesDisplay()
      : module{nullptr},
        fontPath{asset::system("res/fonts/ShareTechMono-Regular.ttf")} {}

  // TODO: We'll want a Framebuffer for this at some point?
  // As of July 29, 2026, the unbuffered drawLayer is consuming less than
  // 40ms every 40 seconds (when running at 30fps). I'll check this
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
      float fuzz = 0.0f;
      bool fuzz_uniform = false;
      if (module) {
        // Get values from actual module.
        for (int i = 0; i < Chances::PAIR_COUNT; ++i) {
          values[i] = module->prev_values[i];
          counts[i] = module->prev_counts[i];
        }
        out_channels =
            std::max(1, module->outputs[Chances::OUT_OUTPUT].getChannels());
        for (int c = 0; c < out_channels; ++c) {
          current_outs[c] = module->last_output_value[c];
        }
        fuzz = module->params[Chances::SPREAD_PARAM].getValue();
        fuzz_uniform =
            module->params[Chances::SPREAD_TYPE_PARAM].getValue() > 0.5;
      } else {
        // Default values to show in module browser and library.
        values[0] = -1.5;
        counts[0] = 10;
        values[1] = 0.5;
        counts[1] = 16;
        values[2] = 1.2;
        counts[2] = 5;
        out_channels = 1;
        current_outs[0] = 0.5;
        fuzz = 0.25;
        fuzz_uniform = false;
      }

      Rect r = box.zeroPos();
      Vec bounding_box = r.getBottomRight();

      // Since values can be the same on multiple knobs, we need to combine them
      // and sort them by value (map does the sorting).
      // TODO: this map would only change when the knobs change. And process()
      // recomputes it when the knobs move. Maybe just grab it from there?
      // Not sure how to do in a thread-safe way.
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
        min_val = aggregated_counts.begin()->first - fuzz;
        max_val = aggregated_counts.rbegin()->first + fuzz;
        range = max_val - min_val;
      }

      // We should display at least one voltage indicator at all times, so make
      // the range slightly larger than one volt.
      range = std::max(range, 1.2f);

      // Pre-compute curve heights and overall peak height across all overlaps.
      float peak_height = static_cast<float>(max_count);
      float drawable_width = bounding_box.x - rect_width;
      static constexpr int MAX_STEPS = 512;
      int num_steps =
          std::min(static_cast<int>(std::ceil(bounding_box.x)), MAX_STEPS - 1);
      float curve_heights[MAX_STEPS] = {0.0f};

      // In case there's a curve, we need to compute the curve before we draw
      // anything, because the curve might extend above our assumed peak_height
      // and thus our Y dimension.
      if (max_count > 0 && fuzz > 0.0f) {
        // There is actually a spread curve to draw.
        // Iterate through each pair and accumulate its contribution only across
        // the pixel span it actually covers [val - fuzz, val + fuzz].
        for (const auto& pair : aggregated_counts) {
          float val = pair.first;
          float count = pair.second;

          // Convert [val - fuzz, val + fuzz] to display pixel index bounds.
          float min_x = (rect_width / 2.0f) +
                        ((val - fuzz - min_val) / range) * drawable_width;
          float max_x = (rect_width / 2.0f) +
                        ((val + fuzz - min_val) / range) * drawable_width;

          int i_start = std::max(0, static_cast<int>(std::floor(min_x)));
          int i_end = std::min(num_steps, static_cast<int>(std::ceil(max_x)));

          for (int i = i_start; i <= i_end; ++i) {
            if (fuzz_uniform) {
              curve_heights[i] += count;
            } else {
              float voltage =
                  (i - (rect_width / 2.0f)) / drawable_width * range + min_val;
              float distance = voltage - val;
              // Exact PDF of Irwin-Hall distribution for n=3 (sum of 3
              // uniforms): In process(), noise is:
              //   ((u1 + u2 + u3)/3 - 0.5) * 2 * fuzz
              // S = (u1 + u2 + u3) ranges from [0, 3], centered at 1.5.
              // Map distance in [-fuzz, +fuzz] back to s in [0, 3]:
              float s = (distance + fuzz) * (3.0f / (2.0f * fuzz));
              s = clamp(s, 0.0f,
                        3.0f);  // Guard against float rounding at edges.
              float curve_addition = 0.0f;
              // Standard piecewise quadratic equation for Irwin-Hall (n=3):
              if (s <= 1.0f) {
                // Left tail: [0, 1]
                curve_addition = 0.5f * s * s;
              } else if (s <= 2.0f) {
                // Center hump: [1, 2], peak is at s = 1.5 where f = 0.75
                // 0.5 * (-2s^2 + 6s - 3)
                curve_addition = 0.5f * (-2.0f * s * s + 6.0f * s - 3.0f);
              } else {
                // Right tail: [2, 3]
                float d = s - 3.0f;
                curve_addition = 0.5f * d * d;
              }
              // Since f(1.5) = 0.75, multiply by (4/3) to normalize peak
              // to 1.0, then multiply by count so the peak height matches the
              // anchor count.
              curve_heights[i] += count * (curve_addition * (4.0f / 3.0f));
            }
          }
        }

        // In case the accumulated curve pops above the tent poles, find the
        // highest peak across the entire curve.
        for (int i = 0; i <= num_steps; ++i) {
          if (curve_heights[i] > peak_height) {
            peak_height = curve_heights[i];
          }
        }
      }

      float display_max_count = peak_height;

      int y_interval = 1;
      int rounded_max_count = static_cast<int>(std::ceil(display_max_count));
      if (rounded_max_count >= 500)
        y_interval = 100;
      else if (rounded_max_count >= 200)
        y_interval = 50;
      else if (rounded_max_count >= 80)
        y_interval = 25;
      else if (rounded_max_count >= 35)
        y_interval = 10;
      else if (rounded_max_count >= 16)
        y_interval = 5;
      else if (rounded_max_count >= 8)
        y_interval = 2;

      // Make sure background lines and labels are not drawn outside the
      // display.
      nvgScissor(args.vg, RECT_ARGS(r));

      // Draw faint grid background.
      nvgBeginPath(args.vg);
      // Horizontal lines (Y-axis / counts).
      if (display_max_count > 0.0f) {
        for (int c = y_interval; c <= display_max_count; c += y_interval) {
          float y =
              bounding_box.y -
              (static_cast<float>(c) / display_max_count) * bounding_box.y;
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

      // Draw continuous PDF curve, if applicable.
      if (display_max_count > 0.0f && fuzz > 0.0f) {
        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, 0, bounding_box.y);

        for (int i = 0; i <= num_steps; ++i) {
          float x = i;
          float y = bounding_box.y -
                    (curve_heights[i] / display_max_count) * bounding_box.y;
          nvgLineTo(args.vg, x, y);
        }
        nvgLineTo(args.vg, bounding_box.x, bounding_box.y);
        nvgClosePath(args.vg);

        // Semi-transparent blueish glow
        nvgFillColor(args.vg, nvgRGBA(150, 150, 250, 150));
        nvgFill(args.vg);
      }

      // Draw rectangles for exact anchors.
      if (max_count > 0) {
        for (const auto& pair : aggregated_counts) {
          float val = pair.first;
          int count = pair.second;

          float mapped_x;
          if (range > 0.0f) {
            // Map value from [min_val, max_val] to fit within the box width
            // We subtract rect_width from bounding box so the edges don't clip.
            mapped_x = (rect_width / 2.0f) +
                       ((val - min_val) / range) * drawable_width;
          } else {
            // If all values are the same, draw in the center
            mapped_x = bounding_box.x / 2.0f;
          }

          // Center the rectangle on the mapped X coordinate
          float x = mapped_x - (rect_width / 2.0f);

          float height =
              (static_cast<float>(count) / display_max_count) * bounding_box.y;
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
          // range.
          if (h_val >= min_val && h_val <= max_val) {
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

      // Draw text labels for integer voltages (X-axis).
      std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
      if (font && max_count > 0 && range > 0.0f) {
        nvgSave(args.vg);
        nvgFontSize(args.vg, 11);
        nvgFontFaceId(args.vg, font->handle);
        nvgTextLetterSpacing(args.vg, -1);
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);

        // NanoVG does not have a built-in NVG_DIFFERENCE enum, so we
        // synthesize it. By using custom blend functions and multiplying
        // the source (white text) by (1 - destination color), it perfectly
        // inverts whatever is underneath the text. This is
        // a good reason to draw the text *after* everything else is drawn.
        nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR,
                                    NVG_ONE_MINUS_SRC_COLOR);
        nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 255));

        int start_v = std::floor(min_val);
        int end_v = std::ceil(max_val);
        for (int v = start_v; v <= end_v; ++v) {
          float mapped_x =
              (rect_width / 2.0f) + ((v - min_val) / range) * drawable_width;
          nvgText(args.vg, mapped_x, 2, std::to_string(v).c_str(), NULL);
        }

        // Text labels for counts (Y-axis).
        nvgTextAlign(args.vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
        for (int c = y_interval; c <= display_max_count; c += y_interval) {
          float y =
              bounding_box.y -
              (static_cast<float>(c) / display_max_count) * bounding_box.y;
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
      if (frame_count >= 500) {
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

struct ChancesSmallKnob : RoundSmallBlackKnob {
  ChancesDisplay* display = nullptr;
  int pair_index = -1;

  void onEnter(const EnterEvent& e) override {
    RoundSmallBlackKnob::onEnter(e);
    if (display) display->hovered_pair = pair_index;
  }

  void onLeave(const LeaveEvent& e) override {
    RoundSmallBlackKnob::onLeave(e);
    if (display && display->hovered_pair == pair_index) {
      display->hovered_pair = -1;
    }
  }
};

struct ChancesWidget : ModuleWidget {
  static constexpr float X_DIFF_MM = 11.5;

  explicit ChancesWidget(Chances* module) {
    setModule(module);
    setPanel(
        createPanel(asset::plugin(pluginInstance, "res/Chances.svg"),
                    asset::plugin(pluginInstance, "res/Chances-dark.svg")));

    ChancesDisplay* display =
        createWidget<ChancesDisplay>(mm2px(Vec(1.9, 11.5)));
    display->box.size = mm2px(Vec(72.0, 29.0));
    display->module = module;
    addChild(display);

    // The value-count pairs.
    for (int pos = 0; pos < 5; ++pos) {
      ChancesSmallKnob* ck_count = createParamCentered<ChancesSmallKnob>(
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
      ChancesSmallKnob* ck_count = createParamCentered<ChancesSmallKnob>(
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

    addParam(createParamCentered<CKSS>(mm2px(Vec(8.032, 92.0)), module,
                                       Chances::SPREAD_TYPE_PARAM));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.0, 92.0)), module,
                                                 Chances::SPREAD_PARAM));
    addParam(
        createLightParamCentered<VCVLightButton<MediumSimpleLight<WhiteLight>>>(
            mm2px(Vec(53.0, 92.0)), module, Chances::SORT_PARAM,
            Chances::SORT_LIGHT));

    addParam(
        createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(
            mm2px(Vec(22.0, 116.0)), module, Chances::CONTINUOUS_BUTTON_PARAM,
            Chances::CONTINUOUS_BUTTON_LIGHT));

    RoundBlackKnob* style_knob = createParamCentered<RoundBlackKnob>(
        mm2px(Vec(38.0, 116.0)), module, Chances::STYLE_PARAM);
    style_knob->minAngle = -0.28f * M_PI;
    style_knob->maxAngle = 0.28f * M_PI;
    addParam(style_knob);

    addInput(createInputCentered<ThemedPJ301MPort>(
        mm2px(Vec(68.819, 92.0)), module, Chances::RESET_INPUT));

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

    static const std::pair<std::string, int> input_ranges[] = {
        {"[-5V, 5V]", 0}, {"[0V, 10V]", 1}, {"[-10V, 10V]", 2}};

    menu->addChild(
        createSubmenuItem("Input Selection Range", "", [=](Menu* menu) {
          for (auto line : input_ranges) {
            menu->addChild(createCheckMenuItem(
                line.first, "",
                [=]() { return line.second == module->input_range; },
                [=]() { module->input_range = line.second; }));
          }
        }));
    menu->addChild(createSubmenuItem(
        "Default number of OUT channels",
        string::f("%d", module->default_out_channel_count), [=](Menu* menu) {
          for (int c = 1; c <= PORT_MAX_CHANNELS; c++) {
            std::string channelsLabel = string::f("%d", c);
            menu->addChild(createCheckMenuItem(
                channelsLabel, "",
                [=]() { return module->default_out_channel_count == c; },
                [=]() { module->default_out_channel_count = c; }));
          }
        }));
  }
};

Model* modelChances = createModel<Chances, ChancesWidget>("Chances");
