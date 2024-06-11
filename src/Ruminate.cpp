#include <cmath>

#include "plugin.hpp"
#include "buffered.hpp"

struct Ruminate : PositionedModule {
	enum ParamId {
		BOUNCE_PARAM,
		SPEED_PARAM,
		ADJUST_PARAM,
		PLAY_BUTTON_PARAM,
		INIT_POSITION_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		PLAY_GATE_INPUT,
		SPEED_INPUT,
		ABS_POSITION_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		NOW_POSITION_OUTPUT,
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		BOUNCE_LIGHT,
		CONNECTED_LIGHT,
		PLAY_BUTTON_LIGHT,
		LIGHTS_LEN
	};

  enum PlayState {
    // We have a few states we could be in.
		ADJUSTING,  // * Not playing, but actively moving.
		NO_PLAY,  	// * Not playing at all.
		FADE_UP,  	// * Starting to play.
		PLAYING,   	// * Continuing to play.
		FADE_DOWN		// * Fading out the playing.
		    				// * And back to not recording at all.

		// When starting and stopping the head and fade_on_move is set, the sequence is:
		// * NO_PLAY -> FADE_UP -> PLAYING -> FADE_DOWN -> NO_PLAY.
		// If user starts adjusting in FADE_UP or PLAYING, we move to FADE_DOWN and then NO_PLAY.
		// If user is adjusting when in NO_PLAY, we move to ADJUSTING, and stay there until
		// user stops adjusting.
    //
		// When starting and stopping the head and fade_on_move is clear, the sequence is:
		// * NO_PLAY -> FADE_UP -> PLAYING -> FADE_DOWN -> NO_PLAY.
		// If user starts adjusting in FADE_UP or PLAYING, we adjust the position, but keep doing what we're doing.
		// If user is adjusting when in NO_PLAY, we move the play head accordingly.
		// Should never be in ADJUSTING.
    //
	};

	const double FADE_INCREMENT = 0.02;

	// We look for the nearest Memory every NN samples. This saves CPU time.
  int find_memory_countdown = 0;
  std::shared_ptr<Buffer> buffer;

	// Where we are in the movement, before taking POSITION parameters into account.
	// Always 0.0 <= playback_position < length when Looping.
	// Always 0.0 <= playback_position < 2 * length when Bouncing.
  double playback_position;

  // Haven't started playing yet, use initial position knob.
  bool use_initial_position;

  // Where we are in memory (for the timestamp indicator).
  double display_position;

   // To detect when the ABS_POSITION_PARAM changes.
	double prev_abs_position;
	bool abs_changed; // Set when we detect movement, but only cleared when we use it.

  dsp::SchmittTrigger playTrigger;

  // To display timestamps correctly.
	double seconds = 0.0;
	int length = 0;

	// To fade volume when near a recording head.
	double fade = 1.0f;

	// When fading at the start or end of adjustments.
	double play_fade = 1.0;
	PlayState play_state;
	bool fade_on_move = true;  // Saved in the patch.
	bool speed_is_voct = false;  // Saved in the patch.

	Ruminate() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(BOUNCE_PARAM, 0, 1, 0, "Endpoint Behavior",
								 {"Loop around", "Bounce"});
	  configParam(SPEED_PARAM, -10.f, 10.f, 1.f, "Playback speed/direction");
		configParam(ADJUST_PARAM, -10.f, 10.f, 0.f, "Slider to manually move this playback head within Memory");
		configSwitch(PLAY_BUTTON_PARAM, 0, 1, 0, "Press to start/stop this playback head",
	               {"Silent", "Playing"});
		configParam(INIT_POSITION_PARAM, 0.f, 10.f, 0.f, "Initial position (0 - 10V) when loading patch");

		configInput(ABS_POSITION_INPUT, "Resets position when changed; 0V -> bottom, 10V -> top,");
		configInput(SPEED_INPUT, "Playback speed (added to knob value)");
		configInput(PLAY_GATE_INPUT, "Gate to start/stop playing");
	
		configOutput(NOW_POSITION_OUTPUT, "Point in Memory (0 - 10V) playback head is currently reading,");
		configOutput(LEFT_OUTPUT, "Left");
		configOutput(RIGHT_OUTPUT, "Right");

    line_record.position = 0.0;
		line_record.type = RUMINATE;
		prev_abs_position = -20.0;
		abs_changed = false;
		play_state = NO_PLAY;
		playback_position = -1;
    use_initial_position = true;
	}
  
  const float octaves[8] = {-2, -1, -.5, -.25, .25, .5, 1, 2};
  const float notes[7] = {1, 9.0/8.0, 5.0/4.0, 4.0/3.0, 3.0/2.0, 5.0/3.0, 15.0/8.0};

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "fade_on_move", json_integer(fade_on_move ? 1 : 0));
    json_object_set_new(rootJ, "speed_is_voct", json_integer(speed_is_voct ? 1 : 0));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* fadeJ = json_object_get(rootJ, "fade_on_move");
    if (fadeJ) {
      fade_on_move = json_integer_value(fadeJ) == 1;
    }
    json_t* speedJ = json_object_get(rootJ, "speed_is_voct");
    if (speedJ) {
      speed_is_voct = json_integer_value(speedJ) == 1;
    }
  }

  // Overriding solely to make sure Adjust isn't left in a non-zero state.
	// And also, we should randomize the position.
	void onRandomize(const RandomizeEvent& e) override {
		// Randomize all parameters
		Module::onRandomize(e);

		// Randomize custom state variables.
		params[ADJUST_PARAM].setValue(0.0f);
		// For fun, instead of picking a completely random speed, let's pick from a just
		// intonation scale.
		// Reference: https://en.m.wikipedia.org/wiki/Just_intonation#Diatonic_scale
    params[SPEED_PARAM].setValue(octaves[(int) (random::uniform() * 8)] * 
		                             notes[(int) (random::uniform() * 7)]);
		if (length > 0) {
			playback_position = (int) (((double) length) * random::uniform());
		}
	}

	void process(const ProcessArgs& args) override {
		// Only call this only every N samples, since the vast majority of
		// the time this won't change.
		// The number of modules it needs to go through does seem to increase the
		// CPU consummed by the module.
		if (--find_memory_countdown <= 0) {
      // One hundredth of a second.
      find_memory_countdown = (int) (args.sampleRate / 100);

			buffer = findClosestMemory(getLeftExpander().module);
		}

		bool connected = (buffer != nullptr) && buffer->IsValid();

		int loop_type = (int) params[BOUNCE_PARAM].getValue();
		// If connected and buffer isn't empty.
		if (connected) {
			// These help the Timestamps UI widgets on this module.
			// While we could have Timestamp only pick these up from the Buffer,
			// This means that disconnecting the module doesn't zero-out the
			// Timestamp displays.
			// Bad things happen if these are zero, which sometimes happens on startup.
			length = std::max(buffer->length, 10);
			seconds = std::max(buffer->seconds, 0.1);

			// User (or input) is adjusting the position.
			if (inputs[ABS_POSITION_INPUT].getVoltage() != prev_abs_position) {
				if (prev_abs_position == -20.0) {
					// Getting an initial value, if any.
					prev_abs_position = inputs[ABS_POSITION_INPUT].getVoltage();
				} else {
					prev_abs_position = inputs[ABS_POSITION_INPUT].getVoltage();
					abs_changed = true;
				}
			}

			float slider_value = params[ADJUST_PARAM].getValue();
			// Is our position being adjusted by the slider and/or the position input?
			bool adjusting = abs_changed ||
			    std::fabs(slider_value) > std::numeric_limits<float>::epsilon(); // i.e., is not zero.

			// Are we being told to play?
			playTrigger.process(rescale(
					inputs[PLAY_GATE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
			bool playing = (!adjusting || !fade_on_move) && 
			               ((params[PLAY_BUTTON_PARAM].getValue() > 0.1f) || playTrigger.isHigh());

			// fade_on_move may have just been changed. Make sure that if it's been cleared, we are not in
			// ADJUSTING.
			if (!fade_on_move && play_state == ADJUSTING) {
				play_state = ((params[PLAY_BUTTON_PARAM].getValue() > 0.1f) || playTrigger.isHigh())
				    ? PLAYING : NO_PLAY;
				play_fade = 1.0;
			} 
			
      // Now that we understand our inputs, let's determine what PlayState should be.
			// Take into account when the user starts or stops adjusting, and whether or not we fade when adjusting.
			if (play_state == NO_PLAY && adjusting && fade_on_move) {
				play_state = ADJUSTING;
			} else if (play_state == ADJUSTING && !adjusting) {
				play_state = NO_PLAY;
			}

			switch (play_state) {
				case NO_PLAY:
				case FADE_DOWN: {
					if (playing && (!adjusting || !fade_on_move)) {
						play_state = FADE_UP;
					}
				}
				break;
				case FADE_UP:
				case PLAYING:  {
					if (!playing || (adjusting && fade_on_move)) {
						play_state = FADE_DOWN;
					}
				}
				break;
				case ADJUSTING:
				break;
			}

			// Now set the record_fade value appropriately, which may also affect the state.
			if (play_state == FADE_UP) {
				if (play_fade < 1.0) {
					play_fade = std::min(play_fade + FADE_INCREMENT, 1.0);
				} else {
					play_state = PLAYING;
				}
			} else if (play_state == FADE_DOWN) {
				if (play_fade > 0.0) {
					play_fade = std::max(play_fade - FADE_INCREMENT, 0.0);
				} else {
					play_state = NO_PLAY;
				}
			}

			// This is all to figure out the next position in the memory to go to.
			// Want user to see what initial position we are in, even if not moving yet.
			if (use_initial_position) { // Haven't started yet.
				// Value of "start playing position indicator".
				playback_position = (int) (params[INIT_POSITION_PARAM].getValue() * length / 10.0);
			}
			// We're still moving, either forward or because user is adjusting.
			// 'movement' is combination of speed input and speed param.
			// NB: in v/oct case, we subtract the default 1.0 value for SPEED_PARAM.
			double speed = speed_is_voct ?
			    std::pow(2.0, inputs[SPEED_INPUT].getVoltage() + params[SPEED_PARAM].getValue() - 1.0) :
					inputs[SPEED_INPUT].getVoltage() + params[SPEED_PARAM].getValue();
			double movement = play_state == NO_PLAY ? 0.0 : speed;
			if ((play_state == ADJUSTING) || (!fade_on_move && adjusting)) {
				// Even if we're not playing, we want to show movement caused by POSITION movement,
				// so user can see where playback will pick up.
				// 
				// Either the Adjust slider is non-zero or the ABS POSITION input has changed.
				// we'll let the human slider override the ABS input.
				if (std::fabs(slider_value) > std::numeric_limits<float>::epsilon()) {
					// i.e., is not zero.
					// zero -> no movement.
					// 10 -> move entirety of length of buffer in two seconds.
					movement = (slider_value / 20.0) * length / args.sampleRate;
				} else {
					// We'll just move directly to the specified spot.
					double abs = inputs[ABS_POSITION_INPUT].getVoltage();
					// Correct for values outside of 0-10.
					while (abs < 0.0) {
						abs += 10.0;
					}
					while (abs > 10.0) {
						abs -= 10.0;
					}
					playback_position = (abs * length / 10.0);
					movement = 0.0;
					abs_changed = false;
				}
			}
			
			if (use_initial_position && std::fabs(movement >= std::numeric_limits<float>::epsilon())) {
				use_initial_position = false;
			}
			playback_position += movement;

			// Fix the position, now that the movement has occured.
			switch (loop_type) {
				case 0: {  // Loop around.
					if (playback_position < 0.0) {
						playback_position += length;
					} else if (playback_position >= length) {
						playback_position -= length;
					}
				}
				break;
				case 1: {  // Bounce.
					if (playback_position < 0.0) {
						playback_position += 2 * length;
					} else if (playback_position >= 2 * length) {
						playback_position -= 2 * length;
					}
				}
				break;
				
			}

			display_position = playback_position;

			while (display_position > 2 * length) {
				display_position -= 2 * length;
			}

			if (display_position >= length) {
				switch (loop_type) {
					case 0: {  // Loop around.
						display_position -= length;
					}
					break;
					case 1: {  // Bounce.
						// There might be simpler math for this, it just escapes me now.
						if (display_position < 2 * length) {
							display_position = std::max(0.0, length * 2 - display_position - 1);
						} else {
							display_position -= length * 2;
						}
					}
					break;
				}
			}

			outputs[NOW_POSITION_OUTPUT].setVoltage(display_position * 10.0 / length);
			line_record.position = display_position;

			if (play_state != NO_PLAY && play_state != ADJUSTING) {
				use_initial_position = false;
				// Determine values to emit.
				double closest_head_distance = buffer->NearHead(display_position);
				if (closest_head_distance <= FADE_DISTANCE) {
					// value of fade is simply a measure of how close we are.
					// Don't let it get above 1.0.
					fade = std::min(1.0, closest_head_distance / FADE_DISTANCE);
				} else {
					fade = 1.0;
				}

				FloatPair gotten;
				buffer->Get(&gotten, display_position);

				// If the values we're outputting here are at or very close to zero,
				// we could end a fade_out immediately. 
				double left = fade * play_fade * gotten.left;
				double right = fade * play_fade * gotten.right;
				if (play_state == FADE_DOWN && fabs(left) < 0.1 && fabs(right) < 0.1) {
					play_state = NO_PLAY;
					play_fade = 0.0;
				}
				// Sadly, no simple equivalent for FADE_UP -> PLAYING transition.

				outputs[LEFT_OUTPUT].setVoltage(left);
				outputs[RIGHT_OUTPUT].setVoltage(right);
				lights[PLAY_BUTTON_LIGHT].setBrightness(1.0f);
			} else {
				outputs[LEFT_OUTPUT].setVoltage(0.0f);
				outputs[RIGHT_OUTPUT].setVoltage(0.0f);
				lights[PLAY_BUTTON_LIGHT].setBrightness(0.0f);
			}
		} else {
			// Can only be playing if connected.
			lights[PLAY_BUTTON_LIGHT].setBrightness(0.0f);
		}
    lights[BOUNCE_LIGHT].setBrightness(loop_type == 1);
		lights[CONNECTED_LIGHT].setBrightness(connected ? 1.0f : 0.0f);
	}
};

struct NowTimestamp : TimestampField {
	NowTimestamp() {
  }

  Ruminate* module;

  double getPosition() override {
    if (module && module->length > 0 && module->seconds > 0.0) {
			return module->display_position * module->seconds / module->length;
		}
		return 0.00;  // Dummy display value.
  }

	double getSeconds() override {
    if (module && module->seconds > 0.0) {
			return module->seconds;
		}
		return 2.0;
	}
};

struct AdjustSliderRuminate : VCVSlider {
  
	void onDragEnd(const DragEndEvent& e) override {
    getParamQuantity()->setImmediateValue(0.0);
		VCVSlider::onDragEnd(e);
	}
};

struct RuminateWidget : ModuleWidget {
	RuminateWidget(Ruminate* module) {
		setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Ruminate.svg"),
                         asset::plugin(pluginInstance, "res/Ruminate-dark.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(6.035, 14.0)),
                                             module, Ruminate::BOUNCE_PARAM,
                                             Ruminate::BOUNCE_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.035, 97.087)), module, Ruminate::SPEED_INPUT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(19.05, 97.087)), module, Ruminate::SPEED_PARAM));

    addParam(createParamCentered<AdjustSliderRuminate>(mm2px(Vec(6.35, 43.0)),
		   module, Ruminate::ADJUST_PARAM));
  	addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(19.05, 50.8)),
		   module, Ruminate::INIT_POSITION_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.05, 34.396)),
		   module, Ruminate::ABS_POSITION_INPUT));

		// Play button and trigger.
    addParam(createLightParamCentered<VCVLightLatch<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(19.05, 80.0)),
                                             module, Ruminate::PLAY_BUTTON_PARAM,
                                             Ruminate::PLAY_BUTTON_LIGHT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.035, 80.0)), module, Ruminate::PLAY_GATE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(12.7, 65.0)),
		                                           module, Ruminate::NOW_POSITION_OUTPUT));
		// A timestamp is 10 wide.
		NowTimestamp* now_timestamp = createWidget<NowTimestamp>(mm2px(
        Vec(12.7 - (10.0 / 2.0), 69.0)));
    now_timestamp->module = module;
    addChild(now_timestamp);

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.035, 112.0)), module, Ruminate::LEFT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.05, 112.0)), module, Ruminate::RIGHT_OUTPUT));

		ConnectedLight* connect_light = createLightCentered<ConnectedLight>(
			mm2px(Vec(12.7, 3.2)), module, Ruminate::CONNECTED_LIGHT);
    connect_light->pos_module = module;
		addChild(connect_light);
	}

  void appendContextMenu(Menu* menu) override {
    Ruminate* module = dynamic_cast<Ruminate*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolPtrMenuItem("Fade on Move", "",
                                          &module->fade_on_move));
    menu->addChild(createBoolPtrMenuItem("Use Speed as V/Oct", "",
                                          &module->speed_is_voct));
	}

};

Model* modelRuminate = createModel<Ruminate, RuminateWidget>("Ruminate");
