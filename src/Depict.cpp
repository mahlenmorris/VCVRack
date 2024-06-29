#include <thread>

#include "plugin.hpp"

#include "buffered.hpp"

struct Depict : Module {
	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUTS_LEN
	};
	enum LightId {
		CONNECTED_LIGHT,
		LIGHTS_LEN
	};

  std::shared_ptr<Buffer> buffer;

  // Set by process(), read by drawLayer().
	// Tells the UI where to draw the moving lines representing the player and
	// recorder "heads".
  std::vector<LineRecord> line_records;
	// I guess technically this would be close to the 'distance of the right-most
	// module, but I don't know if I want to count on that.
	int max_distance;

	// To do some tasks every NN samples. Some UI-related tasks are not as
  // latency-sensitive as the audio thread, and we don't need to do often.
  // TODO: consider putting these tasks on a background thread...
  int get_line_record_countdown = 0;

	Depict() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		buffer = nullptr;
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
      get_line_record_countdown = (int) (args.sampleRate / 60);

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
					line_records.push_back(dynamic_cast<PositionedModule*>(next_module)->line_record);
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
					line_records.push_back(dynamic_cast<PositionedModule*>(next_module)->line_record);
          // The largest one might actually be to the left of us! Like if were
					// the left-most module.
					max_distance = std::max(max_distance, line_records.back().distance);
				}
				// If we are still in our module list, move to the left.
				auto m = next_module->model;
				if (m == modelMemory) {
					std::shared_ptr<Buffer> found_buffer = dynamic_cast<BufferedModule*>(next_module)->getHandle()->buffer;
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

struct MemoryDepict : Widget {
  Depict* module;

	// Only used when generating an image for the module browser.
  std::vector<LineRecord> dummy_lines = {
    {2.34, SCHEME_RED, RUMINATE, 1},
    {7.9, SCHEME_BLUE, RUMINATE, 2},
    {5.5, SCHEME_ORANGE, EMBELLISH, 3},
    {0.3, SCHEME_PURPLE, EMBELLISH, 4}
	};

  MemoryDepict() {}

	// Only used when making an image for the module browser.
	void FillDummyWaveform(PointBuffer* waveform, bool dummy_data) {
		if (dummy_data) {
			waveform->normalize_factor = 1.0;  // 10.0V maximum.
			waveform->text_factor.assign("10V");
		} else {
			waveform->normalize_factor = 1000;  // 0.01V maximum.
			waveform->text_factor.assign("0.01V");
		}
		// This is just some silly way to make an interesting waveform I came up with.
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

				waveform->points[i][0] = abs(out_l);
				waveform->points[i][1] = abs(out_r);
			} else {
				waveform->points[i][0] = 0.0;
				waveform->points[i][1] = 0.0;
			}
		}
	}

	// By using drawLayer() instead of draw(), this becomes a glowing Depict
	// when the "room lights" are turned down. That seems correct to me.
  void drawLayer(const DrawArgs& args, int layer) override {
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
    Widget::drawLayer(args, layer);
  }
};

struct DepictWidget : ModuleWidget {
	DepictWidget(Depict* module) {
		setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Depict.svg"),
                         asset::plugin(pluginInstance, "res/Depict-dark.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Screen.
		MemoryDepict* display = createWidget<MemoryDepict>(
			mm2px(Vec(2.408, 14.023)));
	  display->box.size = mm2px(Vec(25.665, 109.141));
		display->module = module;
		addChild(display);

		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(15.240, 3.0)),
		             module, Depict::CONNECTED_LIGHT));
	}
};


Model* modelDepict = createModel<Depict, DepictWidget>("Depict");
