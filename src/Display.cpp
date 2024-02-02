#include "plugin.hpp"

#include "buffered.hpp"


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

  // Set by process(), read by drawLayer().
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
