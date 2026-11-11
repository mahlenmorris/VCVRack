#include <cmath>

#include "StochasticTelegraph.hpp"
#include "buffered.hpp"
#include "plugin.hpp"

struct Depict : Module {
  static const int DEFAULT_WIDTH = 6;
  static const int MAX_WIDTH = 50;
  // Difference in width between current module size and the display size within
  // it.
  static constexpr double DISPLAY_MARGIN = 2 * 2.5;

  enum ParamId { PARAMS_LEN };
  enum InputId { INPUTS_LEN };
  enum OutputId { OUTPUTS_LEN };
  enum LightId { CONNECTED_LIGHT, LIGHTS_LEN };

  std::shared_ptr<Buffer> buffer;

  // Width (in "holes") of the whole module. Changed by the resize bar on the
  // right. Saved in the json for the module.
  int width = Depict::DEFAULT_WIDTH;

  // Set by process(), read by drawLayer().
  // Tells the UI where to draw the moving lines representing the player and
  // recorder "heads".
  std::vector<LineRecord> line_records;

  // I guess technically this would be close to the distance of the right-most
  // module, but I don't know if I want to count on that.
  int max_distance = 0;

  // To do some tasks every NN samples. Some UI-related tasks are not as
  // latency-sensitive as the audio thread, and we don't need to do often.
  // TODO: consider putting these tasks on a background thread...
  int get_line_record_countdown = 0;

  Depict() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    buffer = nullptr;
  }

  // Required by STResizeHandle; is called whenever the width changes.
  // Since Depict redraws on every frame anyway, this is a no-op.
  void RedrawText() {}

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "width", json_integer(width));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* widthJ = json_object_get(rootJ, "width");
    if (widthJ) width = json_integer_value(widthJ);
  }

  void process(const ProcessArgs& args) override {
    // This process() method, many times per second:
    // * Walks left and right on the Memory modules, noting the color and
    //   related position they have (if any).
    // * Updates local data structure to note the positions, so that the
    //   MemoryDepict (below) can draw the results.
    // Note that Memory is responsible for telling each module what color it is.
    if (--get_line_record_countdown <= 0) {
      // One sixtieth of a second.
      get_line_record_countdown = (int)(args.sampleRate / 60);

      bool connected = false;
      max_distance = 0;

      // TODO: Drastic, I know. Maybe slightly better to just replace the
      // existing records and hack off any that should be removed?
      line_records.clear();
      // First head to the right.
      Module* next_module = getRightExpander().module;
      while (next_module) {
        if (ModelHasColor(next_module->model)) {
          // Add to line_records.
          line_records.push_back(
              dynamic_cast<PositionedModule*>(next_module)->line_record);
          max_distance = std::max(max_distance, line_records.back().distance);
        }
        // If we are still in our module list, move to the right.
        if (IsNonMemoryEnsembleModel(next_module->model)) {
          next_module = next_module->getRightExpander().module;
        } else {
          break;
        }
      }
      // Now move to the left.
      next_module = getLeftExpander().module;
      while (next_module) {
        if (ModelHasColor(next_module->model)) {
          // Add to line_records.
          line_records.push_back(
              dynamic_cast<PositionedModule*>(next_module)->line_record);
          // The largest one might actually be to the left of us! Like if were
          // the left-most module.
          max_distance = std::max(max_distance, line_records.back().distance);
        }
        // If we are still in our module list, move to the left.
        auto m = next_module->model;
        if (IsMemoryEnsembleModel(m)) {
          std::shared_ptr<Buffer> found_buffer =
              dynamic_cast<BufferedModule*>(next_module)->getHandle()->buffer;
          if (buffer != found_buffer && found_buffer->IsValid()) {
            buffer = found_buffer;
          }
          connected = true;
          // Memory marks the end of the left side anyway.
          break;
        }
        if (IsNonMemoryEnsembleModel(m)) {
          next_module = next_module->getLeftExpander().module;
        } else {
          break;
        }
      }
      if (!connected) {
        buffer = nullptr;  // We've been disconnected, buffer no longer valid.
      }
      lights[CONNECTED_LIGHT].setBrightness(connected ? 1.0f : 0.0f);
    }
  }
};

struct DepictUndoRedoAction : history::ModuleAction {
  int old_width, new_width;

  // This has two unused float parameters in order to match the signature of the
  // ctor that STResizeHandle expects to call whenever the width changes.
  // They aren't used here because we don't have a handle on the left side.
  DepictUndoRedoAction(int64_t id, int old_width, int new_width,
                       float unused_1 = 0.0f, float unused_2 = 0.0f)
      : old_width{old_width}, new_width{new_width} {
    moduleId = id;
    name = "module width change";
  }
  void undo() override {
    Depict* module = dynamic_cast<Depict*>(APP->engine->getModule(moduleId));
    if (module) {
      module->width = this->old_width;
    }
  }

  void redo() override {
    Depict* module = dynamic_cast<Depict*>(APP->engine->getModule(moduleId));
    if (module) {
      module->width = this->new_width;
    }
  }
};

struct MemoryDepict : Widget {
  Depict* module;
  PointBuffer module_browser, default_module;
  bool module_browser_filled = false;
  bool default_module_filled = false;

  // Only used when generating an image for the module browser.
  std::vector<LineRecord> dummy_lines = {
      {2.34, SCHEME_RED, RUMINATE, 1},
      {7.9, SCHEME_BLUE, RUMINATE, 2},
      {5.5, SCHEME_ORANGE, FIXATION, 3, true, 5.5, 2.0},
      {0.3, SCHEME_PURPLE, EMBELLISH, 4}};

  MemoryDepict() : module{nullptr} {}

  // Only used when making an image for the module browser or for a Depict
  // with no valid buffer data yet.
  void FillDummyWaveform(PointBuffer* waveform, bool dummy_data) {
    if (dummy_data) {
      // For module browser, we make something cool looking.
      waveform->normalize_factor = 1.0;  // 10.0V maximum.
      waveform->text_factor.assign("10V");
    } else {
      // Just an empty waveform.
      waveform->normalize_factor = 1000;  // 0.01V maximum.
      waveform->text_factor.assign("0.01V");
    }
    // This is just some silly way to make an interesting waveform for the
    // module browser I came up with.
    double z = 1.0, z2 = 1.0;
    double in2 = 1.133, in3 = 2.024;
    double out = 0.0, out_l, out_r;
    for (int i = 0; i < WAVEFORM_SIZE; ++i) {
      if (dummy_data) {
        z *= .98;
        z2 *= .91;
        if (z < 0.0001) z = 1.0;
        if (z2 < 0.0001) z2 = 1.0;
        out = (sin(out * 5) * in2 * in3 + 0.1) * 3;

        // Make it vary a bit left-to-right, so it doesn't look too symmetrical.
        double bias = sin(6.28318 * i / WAVEFORM_SIZE);  // -1 -> 1
        out_l = (out + bias) * std::max(z, z2);
        out_r = (out - bias) * std::max(z, z2);

        waveform->points[i][0][0] = abs(out_l);
        waveform->points[i][1][0] = abs(out_r);
      } else {
        // Just zeros for left and right.
        waveform->points[i][0][0] = 0.0;
        waveform->points[i][1][0] = 0.0;
      }
    }
  }

  // Draw the black screen background in draw() so it expands cleanly as the
  // module is widened past its default 6 HP SVG screen rect.
  // Doing this here means it doesn't "glow" when the lights are turned down.
  void draw(const DrawArgs& args) override {
    nvgScissor(args.vg, RECT_ARGS(args.clipBox));
    nvgBeginPath(args.vg);
    nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(args.vg, nvgRGB(0, 0, 0));
    nvgFill(args.vg);
    Widget::draw(args);
    nvgResetScissor(args.vg);
  }

  // By using drawLayer() instead of draw(), this becomes a glowing Depict
  // when the "room lights" are turned down. That seems correct to me.
  void drawLayer(const DrawArgs& args, int layer) override {
    if (layer == 1) {
      int max_distance;
      int line_record_size;
      int buffer_length;
      bool is_cv_rate;
      PointBuffer* waveform;

      std::shared_ptr<Buffer> buffer = nullptr;
      if (module) {
        max_distance = std::max(1, module->max_distance + 1);
        line_record_size = (int)module->line_records.size();
        buffer = module->buffer;  // In case it gets reset by another action.
        if (buffer && buffer->IsValid()) {
          waveform = &(buffer->waveform);
          buffer_length = buffer->length;
          is_cv_rate = buffer->cv_rate;
        } else {
          waveform = &default_module;
          if (!default_module_filled) {
            FillDummyWaveform(waveform, false);
            default_module_filled = true;
          }
          buffer_length = 100000;
          is_cv_rate = false;
        }
      } else {
        // Dummy data for the module browser.
        max_distance = 5;  // I'll have four heads.
        line_record_size = 4;
        waveform = &module_browser;
        if (!module_browser_filled) {
          FillDummyWaveform(waveform, true);
          module_browser_filled = true;
        }
        buffer_length = 10;
        is_cv_rate = false;
      }

      // just in case max_distance is zero somehow, I don't want to divide by
      // it.
      Rect r = box.zeroPos();
      Vec bounding_box = r.getBottomRight();

      double x_per_volt = (bounding_box.x - 1.0) / 20.0;
      double zero_volt_right = bounding_box.x / 2 + 0.5;
      double y_per_point = bounding_box.y / WAVEFORM_SIZE;

      // Make half-white.
      nvgFillColor(args.vg, nvgRGBA(140, 140, 140, 255));

      nvgSave(args.vg);
      nvgScissor(args.vg, RECT_ARGS(r));  // Not sure this is right?

      if (is_cv_rate) {
        // Minimum display width at 6 HP in pixels minus the DISPLAY_MARGIN,
        // so that a single channel amplitude stays at a consistent scale as
        // Depict is widened to spread multiple channels apart.
        double min_display_width =
            mm2px(Depict::DEFAULT_WIDTH * 5.08 - Depict::DISPLAY_MARGIN);

        double cv_x_per_volt = (min_display_width - 1.0) / 20.0;
        // The extra space on half of the display from being wider than
        // DEFAULT_WIDTH.
        double extra_half_width =
            std::max(0.0, (bounding_box.x - min_display_width) * 0.5);

        // if doing CV, zero is the middle of the left or right side.
        double zero_volt_mid_left = bounding_box.x * 0.25;
        double zero_volt_mid_right = bounding_box.x * 0.75;
        int l_chans = buffer ? std::max(1, buffer->left_channels) : 1;
        int r_chans = buffer ? std::max(1, buffer->right_channels) : 1;

        auto get_left_zero = [&](int c) -> double {
          if (l_chans <= 1) return zero_volt_mid_left;
          double t = (double)c / (l_chans - 1) - 0.5;
          return zero_volt_mid_left + t * extra_half_width;
        };
        auto get_right_zero = [&](int c) -> double {
          if (r_chans <= 1) return zero_volt_mid_right;
          double t = (double)c / (r_chans - 1) - 0.5;
          return zero_volt_mid_right + t * extra_half_width;
        };

        // We draw vertical oscilliscope signals for CV, since positive and
        // negative values matter more than amplitude.
        nvgStrokeWidth(args.vg, 1);

        // When we're drawing many (probably) overlapping lines, we don't
        // want them all at full brightness. But as the width gets wider, the
        // likelihood that they are overlapping goes down. So we lower the
        // brightness of each line as we draw more channels, but brighten them
        // back toward full white as the channels separate horizontally.
        auto get_channel_alpha = [&](int chans) -> float {
          if (chans <= 1) return 1.0f;  // One channel -> full bright.
          float base_alpha = std::max(0.25f, 1.0f / std::sqrt((float)chans));
          double spacing = extra_half_width / (chans - 1);
          double full_swing = min_display_width * 0.25;
          float sep = (float)std::min(
              1.0, full_swing > 0.0 ? spacing / full_swing : 0.0);
          return base_alpha + (1.0f - base_alpha) * sep;
        };

        // Draw left points on the left of the midline.
        float l_alpha = get_channel_alpha(l_chans);
        nvgStrokeColor(args.vg, nvgTransRGBAf(SCHEME_WHITE, l_alpha));
        for (int c = 0; c < l_chans; ++c) {
          double chan_zero = get_left_zero(c);
          nvgBeginPath(args.vg);
          for (int i = 0; i < WAVEFORM_SIZE; i++) {
            float max = waveform->points[i][0][c] * waveform->normalize_factor;
            float x = chan_zero - (max * cv_x_per_volt / 2);
            float y = (WAVEFORM_SIZE - i) * y_per_point;
            if (i == 0) {
              nvgMoveTo(args.vg, x, y);
            } else {
              nvgLineTo(args.vg, x, y);
            }
          }
          nvgStroke(args.vg);
          nvgClosePath(args.vg);
        }

        // Draw right points on the right of the midline.
        float r_alpha = get_channel_alpha(r_chans);
        nvgStrokeColor(args.vg, nvgTransRGBAf(SCHEME_WHITE, r_alpha));
        for (int c = 0; c < r_chans; ++c) {
          double chan_zero = get_right_zero(c);
          nvgBeginPath(args.vg);
          for (int i = 0; i < WAVEFORM_SIZE; i++) {
            float max =
                waveform->points[i][1][c] * waveform->normalize_factor_right;
            float x = chan_zero - (max * cv_x_per_volt / 2);
            float y = (WAVEFORM_SIZE - i) * y_per_point;
            if (i == 0) {
              nvgMoveTo(args.vg, x, y);
            } else {
              nvgLineTo(args.vg, x, y);
            }
          }
          nvgStroke(args.vg);
          nvgClosePath(args.vg);
        }
        // Draw zero-volt reference lines for L and R channels.
        nvgBeginPath(args.vg);
        for (int c = 0; c < l_chans; ++c) {
          nvgRect(args.vg, get_left_zero(c), 0, 0.5f, bounding_box.y);
        }
        for (int c = 0; c < r_chans; ++c) {
          nvgRect(args.vg, get_right_zero(c), 0, 0.5f, bounding_box.y);
        }
        nvgFillColor(args.vg, nvgRGBA(140, 140, 140, 128));
        nvgFill(args.vg);
        // When wider than default, also draw a subtle divider line down the
        // center between L and R.
        if (extra_half_width > 0.0) {
          nvgBeginPath(args.vg);
          nvgRect(args.vg, bounding_box.x * 0.5 - 0.25f, 0, 0.5f,
                  bounding_box.y);
          // There's a lot of white-grey on the screen already, let's make this
          // a blue.
          nvgFillColor(args.vg, nvgRGBA(20, 20, 150, 200));
          nvgFill(args.vg);
        }
      } else {
        double zero_volt_left = bounding_box.x / 2 - 0.5;
        // Draw the wave forms.
        // In one shape we:
        // * Draw the left side side from bottom to top.
        // * Draw the right side line from top to bottom.
        // * Join them. Draw that shape.
        // Then draw a white line down the middle to suggest that these are
        // two separate channels.
        nvgBeginPath(args.vg);
        // Draw left points on the left of the mid.
        for (int i = 0; i < WAVEFORM_SIZE; i++) {
          float max = waveform->points[i][0][0] * waveform->normalize_factor;
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
          float max = waveform->points[i][1][0] * waveform->normalize_factor;
          float x = zero_volt_right + (max * x_per_volt);
          float y = (WAVEFORM_SIZE - 1 - i) * y_per_point;
          nvgLineTo(args.vg, x, y);
        }

        nvgClosePath(args.vg);
        nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
        nvgFill(args.vg);

        // Draw center line down the middle, separating L and R.
        nvgBeginPath(args.vg);
        nvgRect(args.vg, zero_volt_left, 0, 0.5f, bounding_box.y);
        nvgFillColor(args.vg, SCHEME_WHITE);
        nvgFill(args.vg);
      }

      // Add text to indicate the largest value we currently display.
      nvgBeginPath(args.vg);
      nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
      nvgFillColor(args.vg, SCHEME_BLUE);
      nvgFontSize(args.vg, 11);
      // Do I need this? nvgFontFaceId(args.vg, font->handle);
      nvgTextLetterSpacing(args.vg, -1);

      // Place on the line just off the left edge.
      nvgText(args.vg, 4, 10, waveform->text_factor.c_str(), NULL);

      if (buffer && buffer->cv_rate) {
        // We allow left and right CV to be scaled differently, so we can
        // show detail better.
        // Add text to indicate the largest right-hand value we currently
        // display.
        nvgBeginPath(args.vg);
        // Do I need this? nvgFontFaceId(args.vg, font->handle);
        nvgTextLetterSpacing(args.vg, -1);

        // Place on the line just off the left edge.
        nvgText(args.vg, zero_volt_right + 4, 10,
                waveform->text_factor_right.c_str(), NULL);
      }

      // Draw pale rectangles for any heads that know what will be played (e.g.
      // Fixation in Style 1 or 2).
      for (int i = 0; i < line_record_size; i++) {
        LineRecord line = module ? module->line_records[i] : dummy_lines[i];
        if (line.type == FIXATION && line.has_range && buffer_length > 0) {
          double center = bounding_box.x * line.distance / max_distance;
          double len = bounding_box.x * 0.2;
          double rect_x = center - len / 2.0;
          double rect_w = len;

          nvgBeginPath(args.vg);
          auto drawSegment = [&](double start, double length) {
            if (length <= 0.0) return;
            double p1 = std::max(0.0, std::min(start, (double)buffer_length));
            double p2 =
                std::max(0.0, std::min(start + length, (double)buffer_length));
            double y_bottom = bounding_box.y * (1.0 - (p1 / buffer_length));
            double y_top = bounding_box.y * (1.0 - (p2 / buffer_length));
            nvgRect(args.vg, rect_x, y_top, rect_w, y_bottom - y_top);
          };

          drawSegment(line.range_start, line.range_length);
          drawSegment(line.range_start2, line.range_length2);
          nvgFillColor(args.vg, nvgTransRGBAf(line.color, 0.5f));
          nvgFill(args.vg);
        }
      }

      // Then draw the recording heads on top.
      // Don't draw anything if disconnected from a Memory[CV], unless we are
      // drawing the module browser image.
      for (int i = 0; i < line_record_size; i++) {
        LineRecord line;
        if (module) {
          line = module->line_records[i];
        } else {
          line = dummy_lines[i];
        }
        nvgBeginPath(args.vg);
        // I picture 0.0 at the bottom.
        double y_pos =
            bounding_box.y * (1 - ((double)line.position / buffer_length));
        // Line is changed by distance and type.
        if (line.type == RUMINATE) {
          // Endpoint of line suggests which module it is.
          double len = bounding_box.x * line.distance / max_distance;
          nvgRect(args.vg, 0.0, y_pos, len, 1);
        } else if (line.type == EMBELLISH) {
          double len =
              bounding_box.x * (max_distance - line.distance) / max_distance;
          nvgRect(args.vg, bounding_box.x - len, y_pos, len, 2);
        } else if (line.type == FIXATION) {
          double center = bounding_box.x * line.distance / max_distance;
          // Make these lines one fifth of the way across. Sure they'll
          // overlap sometimes, but they typically don't travel the whole
          // vertical length of the buffer, and they have to show up againt
          // the white waveform.
          double len = bounding_box.x * 0.2;
          nvgRect(args.vg, center - len / 2.0, y_pos, len, 2);
        }
        nvgFillColor(args.vg, line.color);
        nvgFill(args.vg);
      }
      // Restore previous state.
      nvgResetScissor(args.vg);
      nvgRestore(args.vg);
    }
  }
};

struct DepictWidget : ModuleWidget {
  StochasticTelegraph::STResizeHandle<Depict, DepictUndoRedoAction>*
      rightHandle;
  MemoryDepict* display;

  DepictWidget(Depict* module) {
    setModule(module);
    setPanel(createPanel<StochasticTelegraph::RestrictedThemedSvgPanel>(
        asset::plugin(pluginInstance, "res/Depict.svg"),
        asset::plugin(pluginInstance, "res/Depict-dark.svg")));

    // Set reasonable initial size of module. Will likely get updated below.
    box.size = Vec(RACK_GRID_WIDTH * Depict::DEFAULT_WIDTH, RACK_GRID_HEIGHT);
    if (module) {
      // Set box width from loaded Module when available.
      box.size.x = module->width * RACK_GRID_WIDTH;
    } else {
      // Like when showing the module in the module browser.
      box.size.x = Depict::DEFAULT_WIDTH * RACK_GRID_WIDTH;
    }

    // Screen.
    display = createWidget<MemoryDepict>(mm2px(Vec(1.0, 14.023)));
    display->box.size =
        Vec(box.size.x - mm2px(Depict::DISPLAY_MARGIN), mm2px(109.141));
    display->module = module;
    addChild(display);

    // Resize bar on right. Minimum width is DEFAULT_WIDTH (6 HP), max is
    // MAX_WIDTH (50 HP).
    rightHandle =
        new StochasticTelegraph::STResizeHandle<Depict, DepictUndoRedoAction>(
            true, Depict::DEFAULT_WIDTH, Depict::MAX_WIDTH);
    rightHandle->module = module;
    // Make sure the handle is correctly placed if drawing for the module
    // browser.
    rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;
    addChild(rightHandle);

    // Our light is not colored, since Depict doesn't have a position in Depict.
    addChild(createLightCentered<MediumLight<WhiteLight>>(
        mm2px(Vec(15.240, 3.0)), module, Depict::CONNECTED_LIGHT));
  }

  void step() override {
    Depict* module = dynamic_cast<Depict*>(this->module);
    if (module) {
      if (box.size.x != module->width * RACK_GRID_WIDTH) {
        // Module width has changed. Might be first time step() has been called,
        // or undo/redo has happened. But we check for it because when STRIP
        // creates a new instance of this module, it may not pick up the
        // JSON-saved width until after Rack has placed the other modules,
        // causing gaps or overlaps.
        box.size.x = module->width * RACK_GRID_WIDTH;
        // This forces the other modules to the right place if needed.
        APP->scene->rack->setModulePosForce(this, box.pos);
      }
    } else {
      // Like when showing the module in the module browser.
      box.size.x = Depict::DEFAULT_WIDTH * RACK_GRID_WIDTH;
    }
    display->box.size.x = box.size.x - mm2px(Depict::DISPLAY_MARGIN);
    rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;

    ModuleWidget::step();
  }

  void appendContextMenu(Menu* menu) override {
    Depict* module = dynamic_cast<Depict*>(this->module);
    assert(module);

    // Be a little clearer how to make this module do anything.
    menu->addChild(new MenuSeparator);
    menu->addChild(
        createMenuLabel("Depict only works when touching a group of modules "
                        "with a Memory or MemoryCV"));
    menu->addChild(
        createMenuLabel("module to the left. See my User Manual for details "
                        "and usage videos."));
  }
};

Model* modelDepict = createModel<Depict, DepictWidget>("Depict");
