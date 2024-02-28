#include <thread>

#include "plugin.hpp"

#include "buffered.hpp"

// Much of the ideas and names for the waveform drawing are lifted from the
// VCV Scope module.

struct PointBuffer {
	// We just measure the amplitudes, not the min and max of the waves.
	// At the scale we show, a single channel is nearly certain to be symmetric.
	// I.e., we are closer to SoundCloud than Scope.
	float points[WAVEFORM_SIZE][2];
	double normalize_factor;
	std::string text_factor;
};

static const float WIDTHS[] = {
	0.01, 0.02, 0.05,
	0.1, 0.2, 0.5,
	1, 2, 5,
  10, 20, 50};
static const char* TEXTS[] = {
	"0.01V", "0.02V", "0.05V",
	"0.1V", "0.2V", "0.5V",
	"1V", "2V", "5V",
	"10V", "20V", "50V"
};

struct WaveformScanner {
  std::shared_ptr<Buffer> buffer;
	std::shared_ptr<Buffer> next_buffer;
	PointBuffer* points;
	bool shutdown;

	WaveformScanner(std::shared_ptr<Buffer> the_buffer, PointBuffer* the_points) :
	    buffer{the_buffer}, next_buffer{nullptr}, points{the_points},
			shutdown{false} {}

  void Halt() {
		shutdown = true;
	}

  // If this Depict is attached to a different Memeory than was previously the
	// case, we need to act accordingly.
  void UpdateBuffer(std::shared_ptr<Buffer> updated_buffer) {
		if (next_buffer == nullptr && updated_buffer != buffer) {
			next_buffer = updated_buffer;
		}
		// Scan() will actually do the updating.
	}

  // TODO: to make this faster, add a bool array of dirty bits, telling us
	// which sections to scan.
	void Scan() {
		while (!shutdown) {
			int sector_size = buffer->length / WAVEFORM_SIZE;
			float peak_value = 0.0f;

			// We may be pointing to a different Buffer. act accordingly.
			bool full_scan = false;
			if (next_buffer != nullptr) {
				if (next_buffer != buffer) {
					buffer = next_buffer;
					full_scan = true;
				}
				next_buffer = nullptr;
			}

			// For now, do this the most brute-force way; scan from bottom to top.
			for (int p = 0; !shutdown && p < WAVEFORM_SIZE; p++) {
				if (full_scan || buffer->dirty[p]) {
					float left_amplitude = 0.0, right_amplitude = 0.0;
					for (int i = p * sector_size;
						   !shutdown && i < std::min((p + 1) * sector_size, buffer->length);
							 i++) {
						left_amplitude = std::max(left_amplitude,
							                        std::fabs(buffer->left_array[i]));
						right_amplitude = std::max(right_amplitude,
						                           std::fabs(buffer->right_array[i]));
					}
					points->points[p][0] = left_amplitude;
					points->points[p][1] = right_amplitude;
			    // WARN("%d: left = %f, right = %f", p, left_amplitude, right_amplitude);

					// Not stricly speaking correct (the sector may have been written
					// to during the scan). But correctness is not critical, and the new
					// values will be caught if any futher writes to this sector occur,
					// which, since writes are sequential, is quite likely.
					buffer->dirty[p] = false;
				}
				peak_value = std::max(peak_value, points->points[p][0]);
				peak_value = std::max(peak_value, points->points[p][1]);
			}
			float window_size;
			for (int i = 0; i < 12; i++) {
				float f = WIDTHS[i];
				if (peak_value <= f || i == 11) {
					window_size = f;
					points->text_factor.assign(TEXTS[i]);
					break;
				}
			}
			points->normalize_factor = 10.0f / window_size;

			// Pause for a bit to let other threads give us something to do.
			if (!shutdown) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}
	}
};

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

	// [2] -> 0 is left, 1 is right.
  PointBuffer point_buffer;
	WaveformScanner* scanner;
	std::thread* point_refresher;

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
		scanner = nullptr;  // Can't create this until Buffer is found.
	}

	~Depict() {
		if (scanner != nullptr) {
			scanner->Halt();
			if (point_refresher->joinable()) {
				point_refresher->join();
			}
			delete point_refresher;
			delete scanner;
		}
	}

	void process(const ProcessArgs& args) override {
		// This process() method, many times per second:
		// * Walks left and right on the Memory modules, noting the color and
		//   related position they have (if any).
		// * Updates local data structure to note the positions, so that the
		//   MemoryDepict (below) can draw the results.
		// Note that Memory is responsible for telling each module what color it is.
		if (--get_line_record_countdown <= 0) {
      // One hundredth of a second.
      get_line_record_countdown = (int) (args.sampleRate / 100);

      // TODO: Shouldn't these be assigned in Depict() ctor?
			// Or at least create the thread then?
      if (scanner == nullptr) {
				if (buffer != nullptr) {
					scanner = new WaveformScanner(buffer, &point_buffer);
					point_refresher = new std::thread(&WaveformScanner::Scan, scanner);
				}
			}

			bool connected = false;
			max_distance = 0;

      // TODO: Drastic, I know. Maybe slightly better to just replace the
			// existing records and hack off any that should be removed?
			line_records.clear();
			// First head to the right.
			Module* next_module = getRightExpander().module;
			while (next_module) {
				if ((next_module->model == modelRecall) ||
				    (next_module->model == modelEmbellish)) {
					// Add to line_records.
					line_records.push_back(dynamic_cast<PositionedModule*>(next_module)->line_record);
					max_distance = std::max(max_distance, line_records.back().distance);
				}
				// If we are still in our module list, move to the right.
				auto m = next_module->model;
				if ((m == modelRecall) ||
						(m == modelEmbellish) ||
						(m == modelDepict)) {  // This will be a list soon...
					next_module = next_module->getRightExpander().module;
				} else {
					break;
				}
			}
			// Now move to the left.
			next_module = getLeftExpander().module;
			while (next_module) {
				if ((next_module->model == modelRecall) ||
				    (next_module->model == modelEmbellish)) {
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
						// Make sure that we scan the buffer currently connected to us.
						if (scanner != nullptr) {
							scanner->UpdateBuffer(found_buffer);
						}
					}
					connected = true;
					// Memory marks the end of the left side anyway.
					break;
				}
				if ((m == modelRecall) ||
						(m == modelEmbellish) ||
						(m == modelDepict)) {  // This will be a list soon...
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

  MemoryDepict() {}

	// By using drawLayer() instead of draw(), this becomes a glowing Depict
	// when the "room lights" are turned down. That seems correct to me.
  void drawLayer(const DrawArgs& args, int layer) override {
    if ((layer == 1) && module) {
			std::shared_ptr<Buffer> buffer = module->buffer;  // In case it gets reset by another action.
			if (buffer && buffer->IsValid()) {

				// just in case max_distance is zero somehow, I don't want to divide by it.
				int max_distance = std::max(1, module->max_distance + 1);
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
					float max = module->point_buffer.points[i][0] * module->point_buffer.normalize_factor;
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
					float max = module->point_buffer.points[i][1] * module->point_buffer.normalize_factor;
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
				nvgText(args.vg, 4, 10, module->point_buffer.text_factor.c_str(), NULL);

				// Restore previous state.
				nvgResetScissor(args.vg);
				nvgRestore(args.vg);

				// Then draw the recording heads on top.
				for (int i = 0; i < (int) module->line_records.size(); i++) {
					LineRecord line = module->line_records[i];
					nvgBeginPath(args.vg);
					// I picture 0.0 at the bottom. TODO: is that a good idea?
					double y_pos = bounding_box.y *
					               (1 - ((double) line.position / buffer->length));
					// Line is changed by distance and type.
					if (line.type == RECALL) {
						// Endpoint of line suggests which module it is.
						double len = bounding_box.x * line.distance / max_distance;
						nvgRect(args.vg, 0.0, y_pos, len, 1);
					} else {
						double len = bounding_box.x * (max_distance - line.distance) / max_distance;
						nvgRect(args.vg, bounding_box.x - len, y_pos, len, 2);
					}
					nvgFillColor(args.vg, line.color);
					nvgFill(args.vg);
		    }
			}
		}
    Widget::drawLayer(args, layer);
  }
};

struct DepictWidget : ModuleWidget {
	DepictWidget(Depict* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Depict.svg")));

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
