#include <thread>

#include "plugin.hpp"

#include "buffered.hpp"

// Much of the ideas and names for the waveform drawing are lifted from the
// VCV Scope module.
// Number of lines in the waveforms.
// TODO: Test and see if this number is too large.
static const int WAVEFORM_SIZE = 1024;
struct Point {
	float min = INFINITY;
	float max = -INFINITY;
};

struct PointBuffer {
	Point points[WAVEFORM_SIZE][2];
};

struct WaveformScanner {
  Buffer* buffer;
	PointBuffer* points;
	bool shutdown;

	WaveformScanner(Buffer* the_buffer, PointBuffer* the_points) :
	    buffer{the_buffer}, points{the_points}, shutdown{false} {}


  void Halt() {
		shutdown = true;
	}

  // TODO: to make this faster, add a bool array of dirty bits, telling us
	// which sections to scan.
	void Scan() {
		while (!shutdown) {
			int point_size = buffer->length / WAVEFORM_SIZE;
			// For now, do this the most brute-force way; scan from bottom to top.
			for (int p = 0; !shutdown && p < WAVEFORM_SIZE; p++) {
				Point left_point, right_point;
				for (int i = p * point_size;
					   !shutdown && i < std::min((p + 1) * point_size, buffer->length);
						 i++) {
					float left = buffer->left_array[i];
					float right = buffer->right_array[i];
					left_point.min = std::min(left_point.min, left);
					left_point.max = std::max(left_point.max, left);
					right_point.min = std::min(right_point.min, right);
					right_point.max = std::max(right_point.max, right);
				}
				points->points[p][0].min = left_point.min;
				points->points[p][0].max = left_point.max;
				points->points[p][1].min = right_point.min;
				points->points[p][1].max = right_point.max;

		    // WARN("%d: min = %f, max = %f", p, left_point.min, left_point.max);

			}
			if (!shutdown) {
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
		}
	}
};


struct Display : Module {
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

  Buffer* buffer;

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

	Display() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		buffer = nullptr;
		scanner = nullptr;  // Can't create this until Buffer is found.
	}

	~Display() {
		scanner->Halt();
		point_refresher->join();
		delete point_refresher;
		delete scanner;
	}

	void process(const ProcessArgs& args) override {
		// This process() method, every so often (like maybe every hundredth of a second):
		// * Walks left and right on the Memory modules, noting the color and
		//   related position they have (if any).
		// Updates local data structure to note the positions, so that the
		// MemoryDisplay (below) can draw the results.
		// Note that Memory is responsible for telling each module what color it is.

		if (--get_line_record_countdown <= 0) {
      // One hundredth of a second.
      get_line_record_countdown = (int) (args.sampleRate / 100);

      if (scanner == nullptr) {
				if (buffer != nullptr) {
					scanner = new WaveformScanner(buffer, &point_buffer);
					point_refresher = new std::thread(&WaveformScanner::Scan, scanner);
				}
			}

			bool connected = false;
			max_distance = 0;

			line_records.clear();
			// First head to the right.
			Module* next_module = getRightExpander().module;
			while (next_module) {
				if ((next_module->model == modelRecall) ||
				    (next_module->model == modelRemember)) {
					// Add to line_records.
					line_records.push_back(dynamic_cast<PositionedModule*>(next_module)->line_record);
					max_distance = std::max(max_distance, line_records.back().distance);
				}
				// If we are still in our module list, move to the right.
				auto m = next_module->model;
				if ((m == modelRecall) ||
						(m == modelRemember) ||
						(m == modelDisplay)) {  // This will be a list soon...
					next_module = next_module->getRightExpander().module;
				} else {
					break;
				}
			}
			// Now to the left.
			next_module = getLeftExpander().module;
			while (next_module) {
				if ((next_module->model == modelRecall) ||
				    (next_module->model == modelRemember)) {
					// Add to line_records.
					line_records.push_back(dynamic_cast<PositionedModule*>(next_module)->line_record);
          // The largest one might actually be to the left of us! Like if were
					// the left-most module.
					max_distance = std::max(max_distance, line_records.back().distance);
				}
				// If we are still in our module list, move to the left.
				auto m = next_module->model;
				if (m == modelMemory) {
					buffer = dynamic_cast<BufferedModule*>(next_module)->getBuffer();
					connected = true;
					// Memory marks the end of the left side anyway.
					break;
				}
				if ((m == modelRecall) ||
						(m == modelRemember) ||
						(m == modelDisplay)) {  // This will be a list soon...
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

struct MemoryDisplay : Widget {
  Display* module;

  MemoryDisplay() {}

	// By using drawLayer() instead of draw(), this becomes a glowing display
	// when the "room lights" are turned down. That seems correct to me.
  void drawLayer(const DrawArgs& args, int layer) override {
    if ((layer == 1) && module && module->buffer && module->buffer->length > 0) {
			// just in case max_distance is zero somehow, I don't want to divide by it.
			int max_distance = std::max(1, module->max_distance + 1);
			Rect r = box.zeroPos();
			Vec bounding_box = r.getBottomRight();

			double x_per_volt = bounding_box.x / 20.0;
			double zero_volt = bounding_box.x / 2;
			double y_per_point = bounding_box.y / WAVEFORM_SIZE;

			// Draw the wave form.
			// TODO: just drawing the left side for now.
			// Make half-white.
			nvgFillColor(args.vg, nvgRGBA(128, 128, 128, 255));

			nvgSave(args.vg);
			nvgScissor(args.vg, RECT_ARGS(r));  // Not sure this is right?
			nvgBeginPath(args.vg);
			// Draw max points on the left.
			for (int i = 0; i < WAVEFORM_SIZE; i++) {
				const Point& point = module->point_buffer.points[i][0];
				float max = point.max;
				if (!std::isfinite(max))
					max = 0.f;
				// We'll say the x position ranges from -10V to 10V.
				float x = zero_volt - (max * x_per_volt);
				float y = (WAVEFORM_SIZE - i) * y_per_point;
				if (i == 0)
					nvgMoveTo(args.vg, x, y);
				else
					nvgLineTo(args.vg, x, y);
			}

			// Draw min points on bottom
			for (int i = WAVEFORM_SIZE - 1; i >= 0; i--) {
				const Point& point = module->point_buffer.points[i][0];
				float min = point.min;
				if (!std::isfinite(min))
					min = 0.f;
				float x = zero_volt - (min * x_per_volt);
				float y = (WAVEFORM_SIZE - 1 - i) * y_per_point;
				nvgLineTo(args.vg, x, y);
			}
			nvgClosePath(args.vg);
			// nvgLineCap(args.vg, NVG_ROUND);
			// nvgMiterLimit(args.vg, 2.f);
			nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
			nvgFill(args.vg);
			nvgResetScissor(args.vg);
			nvgRestore(args.vg);

			// Then draw the recording heads on top.
			for (int i = 0; i < (int) module->line_records.size(); i++) {
				LineRecord line = module->line_records[i];
				nvgBeginPath(args.vg);
				// I picture 0.0 at the bottom. TODO: is that a good idea?
				double y_pos = bounding_box.y *
				               (1 - ((double) line.position / module->buffer->length));
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
    Widget::drawLayer(args, layer);
  }
};

struct DisplayWidget : ModuleWidget {
	DisplayWidget(Display* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Display.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Screen.
		MemoryDisplay* display = createWidget<MemoryDisplay>(
			mm2px(Vec(2.408, 14.023)));
	  display->box.size = mm2px(Vec(25.665, 109.141));
		display->module = module;
		addChild(display);

		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(3.394, 7.56)),
		             module, Display::CONNECTED_LIGHT));
	}
};


Model* modelDisplay = createModel<Display, DisplayWidget>("Display");
