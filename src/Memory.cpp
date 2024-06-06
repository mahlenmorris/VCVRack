#include <thread>
#include <cstdlib> // for strtol

// VCV.
#include "plugin.hpp"
#include "osdialog.h"

// Mine.
#include "buffered.hpp"
#include "smoother.h"
#include "tipsy_utils.h"
#include "NoLockQueue.h"

// WAV/AIFF library.
#include "AudioFile.h"

// We auto-scale the view of the waveform in Depict, but snap to these values
// to make it less confusing.
static const float WIDTHS[] = {
  0.01, 0.02, 0.05,
  0.1, 0.2, 0.5,
  1, 2, 5,
  10, 20, 50};
// The string versions of the WIDTHS above.
static const char* TEXTS[] = {
  "0.01V", "0.02V", "0.05V",
  "0.1V", "0.2V", "0.5V",
  "1V", "2V", "5V",
  "10V", "20V", "50V"
};

static constexpr size_t recvBufferSize{1024 * 64};

///////////////////////////////////////////////////////////////
//
// The various threads of a Memory object need to assign tasks to each other.
// These are the structs for doing that. Each struct corresponds to a single
// NoLockQueue, with a single writer and reader.
// All queues are owned by the module.

struct PrepareTask {
  enum Type {
    LOAD_DIRECTORY_SET,  // str1 is the directory name.
    LOAD_FILE            // str1 is the selected file name, str2 is the directory name.
  };
  Type type;
  std::string str1, str2;
  float* new_left_array;
  float* new_right_array;
  std::vector<std::string>* loadable_files;
  
  PrepareTask(const Type the_type) : type{the_type},
      new_left_array{nullptr}, new_right_array{nullptr}, loadable_files{nullptr} {}

  ~PrepareTask() {
  }

  static PrepareTask* LoadFileTask(const std::string& name, const std::string& directory) {
    PrepareTask* task = new PrepareTask(LOAD_FILE);
    task->str1 = name;
    task->str2 = directory;
    return task;
  }

  static PrepareTask* LoadDirectoryTask(const std::string& directory) {
    PrepareTask* task = new PrepareTask(LOAD_DIRECTORY_SET);
    task->str1 = directory;
    return task;
  }
};

struct WidgetToModuleQueue {
  // Only length 5 because, seriously, the UI can't really add tasks quickly.
  SpScLockFreeQueue<PrepareTask*, 5> tasks;
};

struct ModuleToPrepareQueue {
  // Queue is limited to ten items, just so we cannot get absurdly far behind.
  SpScLockFreeQueue<PrepareTask*, 10> tasks;
};

struct BufferTask {
  enum Type {
    REPLACE_AUDIO  // new_left_array and new_right_array.
  };
  Type type;
  float* new_left_array;
  float* new_right_array;
  int sample_count;
  double seconds;
  
  BufferTask(const Type the_type) : type{the_type},
                                    new_left_array{nullptr}, new_right_array{nullptr} {}

  ~BufferTask() {
    // Don't delete new_(left|right)_array, surely pointed to by something else.
  }

  static BufferTask* ReplaceTask(float* new_left, float* new_right,
                                 int sample_count, double seconds) {
    BufferTask* task = new BufferTask(REPLACE_AUDIO);
    task->new_left_array = new_left;
    task->new_right_array = new_right;
    task->sample_count = sample_count;
    task->seconds = seconds;
    return task;
  }
  
};

struct PrepareToBufferQueue {
  // Queue is limited to ten items, just so we cannot get absurdly far behind.
  SpScLockFreeQueue<BufferTask*, 10> tasks;
};

// Class devoted to handling the lengthy (compared to single sample)
// process of filling or replacing the current Buffer.
// TODO: honestly, Module should be putting messages onto a queue instead of
// directly changing internal variables here.
struct BufferChangeThread {
  BufferHandle* handle;
  PrepareToBufferQueue* prepare_buffer_queue;

  float sample_rate;
  bool shutdown;
  bool initiateFill;
  int seconds;
  bool initiateWipe;
  bool running;  // TRUE if still wiping or initializing.
  
  BufferChangeThread(BufferHandle* the_handle, PrepareToBufferQueue* prepare_buffer_queue) :
      handle{the_handle}, prepare_buffer_queue{prepare_buffer_queue} {
    running = false;
    initiateFill = false;
    initiateWipe = false;
    shutdown = false;
  }

  void Halt() {
    shutdown = true;
  }

  void SetRate(float rate) {
    sample_rate = rate;
  }

  void InitiateFill(int new_seconds) {
    seconds = new_seconds;
    initiateFill = true;
  }

  void RefreshWaveform(std::shared_ptr<Buffer> buffer) {
    int sector_size = buffer->length / WAVEFORM_SIZE;
    float peak_value = 0.0f;
    bool full_scan = buffer->full_scan;

    if (full_scan) {
      buffer->full_scan = false;  // I'm doing the scan now.
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
        buffer->waveform.points[p][0] = left_amplitude;
        buffer->waveform.points[p][1] = right_amplitude;
        // WARN("%d: left = %f, right = %f", p, left_amplitude, right_amplitude);

        // Not stricly speaking correct (the sector may have been written
        // to during the scan). But correctness is not critical, and the new
        // values will be caught if any further writes to this sector occur,
        // which, since writes are sequential, is quite likely.
        buffer->dirty[p] = false;
      }
      peak_value = std::max(peak_value, buffer->waveform.points[p][0]);
      peak_value = std::max(peak_value, buffer->waveform.points[p][1]);
    }
    float window_size;
    for (int i = 0; i < 12; i++) {
      float f = WIDTHS[i];
      if (peak_value <= f || i == 11) {
        window_size = f;
        buffer->waveform.text_factor.assign(TEXTS[i]);
        break;
      }
    }
    buffer->waveform.normalize_factor = 10.0f / window_size;
  }

  void Work() {
    while (!shutdown) {
      std::shared_ptr<Buffer> buffer = handle->buffer;
      if (buffer) {
        if (buffer->smooths.additions.size() > 0) {
          Smooth* item;
          while (buffer->smooths.additions.pop(item)) {
            // Check that creation_time is long enough ago that we're
            // confident that the new section is written.
            if (item->creation_time < 0 || (system::getTime() - item->creation_time > 0.001)) {
        	    smooth(buffer->left_array, buffer->right_array, item->position, 25, buffer->length);
              delete item;
            } else {
              buffer->smooths.additions.push(item);
              break;
            }
	        }
        }

        if (prepare_buffer_queue->tasks.size() > 0) {
          BufferTask* task;
          while (prepare_buffer_queue->tasks.pop(task) && !shutdown) {
            switch (task->type) {
              case BufferTask::REPLACE_AUDIO: {
                // Buffer is unavailable during this interval.
                buffer->length = 0;
                buffer->seconds = 0.0;
                // And mark every sector dirty.
                buffer->full_scan = true;

                if (buffer->left_array != nullptr) {
                  delete buffer->left_array;
                }
                buffer->left_array = task->new_left_array;
                if (buffer->right_array != nullptr) {
                  delete buffer->right_array;
                }
                buffer->right_array = task->new_right_array;

                buffer->length = task->sample_count;
                buffer->seconds = task->seconds;
                delete task;
              }
              break;
            }
          }
        } 

        if (initiateFill) {
          initiateFill = false;
          running = true;
          // Invalidate existing contents, if any.
          buffer->length = 0;
          buffer->seconds = 0;

          int samples = std::round(seconds * sample_rate);
          float* new_left_array = new float[samples];
          float* new_right_array = new float[samples];

          // Note that this data is still random. Needs to be wiped!

          // And mark every sector dirty.
          buffer->full_scan = true;

          if (buffer->left_array != nullptr) {
            delete buffer->left_array;
          }
          buffer->left_array = new_left_array;
          if (buffer->right_array != nullptr) {
            delete buffer->right_array;
          }
          buffer->right_array = new_right_array;

          buffer->length = samples;
          buffer->seconds = seconds;
          running = false;
          // A fill always prompts a wipe, let's just do that here.
          initiateWipe = true;
        }
        if (initiateWipe) {
          running = true;
          initiateWipe = false;
          for (int i = 0; i < buffer->length && !shutdown; ++i) {
            buffer->left_array[i] = 0.0f;
            buffer->right_array[i] = 0.0f;
          }

          // And mark every sector dirty.
          buffer->full_scan = true;
          running = false;
        }

        // Keep this after any wipe or major change we do to the buffer.
        if (buffer->freshen_waveform) {
          RefreshWaveform(buffer);
        }
      }

      // It seems like I need a tiny sleep here to allow join() to work
      // on this thread.
      if (!shutdown) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
    }
  }
};

// Class devoted to handling the very lengthy (compared to single sample)
// process of dealing with file systems.
// Much of the logic is directly lifted from voxglitch's modules.
// All mistakes are mine.
// TODO: honestly, Module should be putting messages onto a queue instead of
// directly changing internal variables here.
struct PrepareThread {
  bool shutdown;
  ModuleToPrepareQueue* module_file_queue;
  PrepareToBufferQueue* prepare_buffer_queue;

  float sample_rate;

  // Indicator to UI that File I/O is happening.
  bool busy;

  PrepareThread(ModuleToPrepareQueue* module_file_queue,
                PrepareToBufferQueue* prepare_buffer_queue) {
    this->module_file_queue = module_file_queue;
    this->prepare_buffer_queue = prepare_buffer_queue;
    shutdown = false;
    busy = false;
  }

  void Halt() {
    shutdown = true;
  }

  void SetRate(float rate) {
    sample_rate = rate;
  }

  void DoDirectoryRead(const std::string& load_folder, std::vector<std::string>* loadable_files) {
    // To make it more obvious that we are reading contents in, we eliminate the current contents.
    if (!loadable_files->empty()) {
      loadable_files->clear();
    }

    busy = true;
		std::vector<std::string> dirList = system::getEntries(load_folder.c_str());

		// "Sort the vector.  This is in response to a user who's samples were being
		// loaded out of order.  I think it's a mac thing." - Voxglitch.
		sort(dirList.begin(), dirList.end());

		for (auto path : dirList)	{
			if ((rack::string::lowercase(system::getExtension(path)) == "wav") ||
				  (rack::string::lowercase(system::getExtension(path)) == ".wav")) {
				loadable_files->push_back(system::getFilename(path));
			}
		}
    busy = false;
  }

  void ConvertFileToSamples(PrepareTask* task, AudioFile<float>* audio_file) {
    assert(task->type == PrepareTask::LOAD_FILE);
    // One goal is to minimize the amount of downtime for the buffer,
    // since the AudioFile -> Buffer process is time consuming for large files.
    // So the conversion happens before we swap in the memory.
    int samples = std::round(audio_file->getLengthInSeconds() * sample_rate);
    int file_samples = audio_file->getNumSamplesPerChannel();
    float* new_left_array = new float[samples];
    float* new_right_array = new float[samples];

    // Now fill from the audio_file. Transforms we need to apply are:
    // * Sample rate may be different from the file and VCV.
    // * We typically range from -10.0 to 10.0, AudioFile ranges from -1.0 to 1.0.
    // * File might have been in mono. The norm seems to be to put a mono signal 
    //   on both channels.
    bool file_is_mono = audio_file->getNumChannels() == 1;
    // TODO: put in an optimization if sample rates are the same. Less math to do.
    double sample_rate_ratio = 1.0 * audio_file->getSampleRate() / sample_rate;
    // This can take a while; definitely let a Halt() call interrupt it.
    for (int i = 0; !shutdown && i < samples; ++i) {
      // Use the linear interpolation that I also use in Buffer::Get().
      double position = i * sample_rate_ratio;
      // std::min is just to make sure FP math doesn't push us past the end of the buffer.
      int playback_start = std::min((int) trunc(position), file_samples - 1);
      int playback_end = trunc(playback_start + 1);
      if (playback_end >= file_samples) {
        playback_end = 0;
      }

      float start_fraction = position - playback_start;
      new_left_array[i] = 10.0 *
       (audio_file->samples[0][playback_start] * (1.0 - start_fraction) +
        audio_file->samples[0][playback_end] * (start_fraction));
      if (file_is_mono) {
        new_right_array[i] = new_left_array[i];
      } else {
        new_right_array[i] = 10.0 * 
         (audio_file->samples[1][playback_start] * (1.0 - start_fraction) +
          audio_file->samples[1][playback_end] * (start_fraction));
      }
    }
    task->new_left_array = new_left_array;
    task->new_right_array = new_right_array;
  }  

  void Work() {
    while (!shutdown) {
      if (module_file_queue->tasks.size() > 0) {
        PrepareTask* task;
        while (module_file_queue->tasks.pop(task) && !shutdown) {
          switch (task->type) {
            case PrepareTask::LOAD_FILE: {
              // WARN("Starting load of %s", task->str1.c_str());
              busy = true;
              AudioFile<float>* audio_file = new AudioFile<float>;

              // SLOW: this call can take many seconds.
              bool worked = audio_file->load(system::join(task->str2, task->str1));
              busy = false;
              // WARN("Completed load of %s", task->str1.c_str());             
              // WARN("samples = %d, seconds = %f", audio_file->getNumSamplesPerChannel(), audio_file->getLengthInSeconds());
              // WARN("sample rate = %d", audio_file->getSampleRate());

              // If worked, need to fill buffer.
              if (worked) {
                // This part can also be slow, since it walks over every sample.
                busy = true;
                ConvertFileToSamples(task, audio_file);
                busy = false;
                // Done with the audio_file.
                int sample_count = audio_file->getNumSamplesPerChannel();
                double seconds = audio_file->getLengthInSeconds();
                delete audio_file;

                // Send task to BufferChangeThread.
                BufferTask* replace_task = BufferTask::ReplaceTask(
                  task->new_left_array, task->new_right_array, sample_count, seconds);  
                if (!prepare_buffer_queue->tasks.push(replace_task)) {
                  delete replace_task;  // Queue is full, shed load.
                }

              } else {
                // TODO: send text saying why it failed to output port.
              }
              delete task;
            }
            break;

            case PrepareTask::LOAD_DIRECTORY_SET: {
              busy = true;
              DoDirectoryRead(task->str1, task->loadable_files);
              busy = false;
              delete task;
            }
            break;
          }
        }
      } 

      // It seems like I need a tiny sleep here to allow join() to work
      // on this thread. I make this sleep longer than for BufferChangeThread, since file system
      // activity is so slow that we can ease up on the CPU.
      if (!shutdown) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
    }
  }
};

struct Memory : BufferedModule {
  enum ParamId {
    WIPE_BUTTON_PARAM,
    SECONDS_PARAM,
    RESET_BUTTON_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    WIPE_TRIGGER_INPUT,
		TIPSY_LOAD_INPUT,
		TIPSY_SAVE_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
		LOAD_TRIGGER_OUTPUT,
		SAVE_TRIGGER_OUTPUT,
		TIPSY_LOGGING_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    WIPE_BUTTON_LIGHT,
    RESET_BUTTON_LIGHT,
    FILE_IO_LIGHT,
    LIGHTS_LEN
  };

  // Initialization happens on startup, but also during a reset.
  bool buffer_initialized = false;
  bool init_in_progress = false;

  // Threads and workers for doing long tasks
  BufferChangeThread* buffer_change_worker;
  std::thread* buffer_change_thread;
  // For tasks that don't directly change the Buffer, but might send changes to the
  // buffer_change_thread. Especially important for changes that take long periods of time.
  PrepareThread* prepare_worker;
  std::thread* prepare_thread;

  // lock-free queues for coordinating actions between the threads.
  WidgetToModuleQueue widget_module_queue;
  ModuleToPrepareQueue module_prepare_queue;
  PrepareToBufferQueue prepare_buffer_queue;

  // For wiping contents.
  dsp::SchmittTrigger wipe_trigger;
  // Keeps lights on buttons lit long enough to see.
  int wipe_light_countdown = 0;

  // For Loading and Saving contents.
  std::string load_folder_name;
  std::vector<std::string> loadable_files;

  std::string loaded_file;

  // Listening to the Tipsy connection for loading files.
  unsigned char load_recv_buffer[recvBufferSize];
  tipsy::ProtocolDecoder load_decoder;

  // We sweep the connected modules every NN samples. Some UI-related tasks are 
  // not as latency-sensitive as the audio thread, and we don't need to do often.
  int assign_color_countdown = 0;

  Memory() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configButton(WIPE_BUTTON_PARAM, "Press to wipe contents to 0.0V");
    configInput(WIPE_TRIGGER_INPUT, "A trigger here wipes contents to 0.0V");
    configParam(SECONDS_PARAM, 1, 1000, 15,
        "Length of Memory in seconds; takes effect on next RESET press");
    // This is really an integer.
    getParamQuantity(SECONDS_PARAM)->snapEnabled = true;
    configButton(RESET_BUTTON_PARAM, "Press to reset length and wipe contents to 0.0V");
		configInput(TIPSY_LOAD_INPUT, "Tipsy text input to load named file");
		configOutput(LOAD_TRIGGER_OUTPUT, "Sends a trigger when file load has completed");
		configInput(TIPSY_SAVE_INPUT, "Tipsy text input to save contents to named file");
		configOutput(SAVE_TRIGGER_OUTPUT, "Sends a trigger when file save has completed");
		configOutput(TIPSY_LOGGING_OUTPUT, "Logging of File events; connect to a TTY TEXT input");

    // Setting an initial Buffer object.
    std::shared_ptr<Buffer> temp = std::make_shared<Buffer>();
    (*getHandle()).buffer.swap(temp);

    buffer_change_worker = new BufferChangeThread(getHandle(), &prepare_buffer_queue);
    buffer_change_thread = new std::thread(&BufferChangeThread::Work, buffer_change_worker);
    prepare_worker = new PrepareThread(&module_prepare_queue, &prepare_buffer_queue);
    prepare_thread = new std::thread(&PrepareThread::Work, prepare_worker);

    load_decoder.provideDataBuffer(load_recv_buffer, recvBufferSize);
  }

  ~Memory() {
    (*getHandle()).buffer.reset();
    if (buffer_change_worker != nullptr) {
      buffer_change_worker->Halt();
      buffer_change_worker->initiateFill = false;
      buffer_change_worker->initiateWipe = false;
      if (buffer_change_thread != nullptr) {
        buffer_change_thread->join();
        delete buffer_change_thread;
      }
      delete buffer_change_worker;
    }

    if (prepare_worker != nullptr) {
      prepare_worker->Halt();
      if (prepare_thread != nullptr) {
        prepare_thread->join();
        delete prepare_thread;
      }
      delete prepare_worker;
    }
  }

  // Save and retrieve menu choice(s).
  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    if (!load_folder_name.empty()) {
      json_object_set_new(rootJ, "load_folder", json_string(load_folder_name.c_str()));
    }
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
     json_t* load_folderJ = json_object_get(rootJ, "load_folder");
    if (load_folderJ) {
      load_folder_name.assign(json_string_value(load_folderJ));
      PrepareTask* task = PrepareTask::LoadDirectoryTask(load_folder_name);
      if (!widget_module_queue.tasks.push(task)) {
        delete task;
      }
    }
  }

  static constexpr int COLOR_COUNT = 7;
	NVGcolor colors[COLOR_COUNT] = {
		SCHEME_RED,
		SCHEME_BLUE,
		SCHEME_ORANGE,
		SCHEME_PURPLE,
		SCHEME_GREEN,
		SCHEME_CYAN,
    SCHEME_WHITE
	};

  void process(const ProcessArgs& args) override {
    // This is just for the initial creation of the buffer.
    // We can't fill the buffer until process() is called, since we don't know
    // what the sample rate is.
    if (!buffer_initialized) {
      if (!init_in_progress) {
        // Confirm that we can read the sample rate before starting a fill.
        // Sometimes during startup, sampleRate is still zero.
        if (args.sampleRate > 1.0) {
          buffer_change_worker->SetRate(args.sampleRate);
          prepare_worker->SetRate(args.sampleRate);

          buffer_change_worker->InitiateFill(params[SECONDS_PARAM].getValue());
          // TODO: Should the worker thread be part of the Buffer itself?
          // Should it just be waiting for the sample rate to be filled, and then
          // start itself?
          init_in_progress = true;
        }
      } else {
        if (!buffer_change_worker->running) {
          // Filling + wiping done.
          init_in_progress = false;
          buffer_initialized = true;
        }
      }
    } else {
      // We don't pay any attention to the buttons, etc, until the buffer is initialized.
      // Some lights are lit by triggers; these enable them to be
      // lit long enough to be seen by humans.
      if (wipe_light_countdown > 0) {
        wipe_light_countdown--;
      }

      if (inputs[TIPSY_LOAD_INPUT].isConnected()) {
        auto decoder_status = load_decoder.readFloat(
            inputs[TIPSY_LOAD_INPUT].getVoltage());
        if (!load_decoder.isError(decoder_status) &&
            decoder_status == tipsy::DecoderResult::BODY_READY &&
            std::strcmp(load_decoder.getMimeType(), "text/plain") == 0) {
          std::string next(std::string((const char *) load_recv_buffer));
          if (next.size() > 0) {
            if (next[0] == '#' && loadable_files.size() > 0) {
              // Do special things when of the form #NNNN 
              size_t start_pos = next.find_first_of("0123456789"); // Find the first digit
              if (start_pos == std::string::npos && loadable_files.size() > 0) {
                // Handle the case where no digits are found.
                // TODO: add this to the logging port.
                // std::cerr << "Error: No integer found in the string." << std::endl;
              } else {
                char* endptr;
                long parsed_long = strtol(next.c_str() + start_pos, &endptr, 10);

                // Check for conversion errors
                if (endptr == next.c_str() + start_pos || *endptr != '\0') {
                  // Handle the case where the conversion failed
                  // TODO: add this to the logging port.
                  // std::cerr << "Error: Could not convert substring to integer." << std::endl;
                } else if (parsed_long > INT_MAX || parsed_long < INT_MIN) {
                  // Handle overflow if the parsed value is outside int range
                  // TODO: add this to the logging port.
                  // std::cerr << "Error: Integer conversion out of range." << std::endl;
                } else {
                  int parsed_int = static_cast<int>(parsed_long); // Cast to int if successful
                  // Now pick the relevant file in the directory.
                  // First make sure in the range.
                  parsed_int = parsed_int % loadable_files.size();

                  // OK, so this isn't the precisely correct queue, but we check it just below,
                  // so more clean to do this.
                  PrepareTask* task = PrepareTask::LoadFileTask(loadable_files[parsed_int], load_folder_name);
                  if (!widget_module_queue.tasks.push(task)) {
                    delete task;
                  }
                }
              }
            } else {
              // Not a number, just a name.
              // TODO: ensure that it's in the directory we're reading from,
              //   as long as this isn't a subpath (e.g., foo/bar.wav).
              // BUG: should not clear the buffer when file not found.
              // OK, so this isn't the precisely correct queue, but we check it just below,
              // so more clean to do this.
              PrepareTask* task = PrepareTask::LoadFileTask(next, load_folder_name);
              if (!widget_module_queue.tasks.push(task)) {
                delete task;
              }
            }
          }
        }
      }

      // Deal with tasks from the Widget (aka, the menu).
      if (widget_module_queue.tasks.size() > 0) {
        PrepareTask* task;
        while (widget_module_queue.tasks.pop(task)) {
          switch (task->type) {
            case PrepareTask::LOAD_FILE: {
              // Make sure we're not dealing with a previous load before starting a new one.
              // TODO: in tasks system, this check will be unneeded, as we're just adding to a queue.
              // TODO: though should make sure that we actually can interrupt a file load with a new load,
              // as the Tipsy automation means we could receive load requests far faster than we
              // can process them. Best to not get stuck waiting for a really slow network file
              // to load.
              // This conception of "loaded_file" makes less sense.
              // TODO: fix this.
              loaded_file = task->str1;
              if (!module_prepare_queue.tasks.push(task)) {
                delete task;
              }
            }
            break;

            case PrepareTask::LOAD_DIRECTORY_SET: {
              task->loadable_files = &loadable_files;
              if (!module_prepare_queue.tasks.push(task)) {
                delete task;
              }
            }
            break;
          }
        }
      } 

      // Periodically assign colors to each connected module's light.
      // We also take this opportunity to add any unknown recording heads to
      // the buffer->record_heads vector.
      // I'd considered putting these tasks on a background thread, but
      // getRightExpander() is a method on Module.
      // And we can't put this in a call to onExpanderChange(), because events
      // several modules down can affect these results.
      if (--assign_color_countdown <= 0) {
        // One hundredth of a second.
        assign_color_countdown = (int) (args.sampleRate / 100);

        std::shared_ptr<Buffer> buffer = getHandle()->buffer;
        if (buffer) {  // Checks for null.
          Module* next_module = getRightExpander().module;
          int color_index = -1;
          int distance = 0;
          bool found_depict = false;
          while (next_module) {
            if ((next_module->model == modelRuminate) ||
                (next_module->model == modelEmbellish)) {
              // Assign a Color.
              distance++;
              color_index = (color_index + 1) % COLOR_COUNT;
              PositionedModule* pos_module = dynamic_cast<PositionedModule*>(next_module);
              pos_module->line_record.color = colors[color_index];
              pos_module->line_record.distance = distance;
              if (next_module->model == modelEmbellish) {
                // Make sure it's on the list of record heads.
                bool found = false;
                for (int i = 0; i < (int) buffer->record_heads.size(); ++i) {
                  if (buffer->record_heads[i].module_id == next_module->getId()) {
                    found = true;
                    break;
                  }
                }
                if (!found) {
                  buffer->record_heads.push_back(RecordHeadTrace(next_module->getId(),
                                                      pos_module->line_record.position));
                }
              }
            }
            // If we are still in our module list, move to the right.
            auto m = next_module->model;
            if (m == modelDepict) {
              // If there is a Depict, then make sure the waveform is being updated.
              found_depict = true;
            }
            if ((m == modelRuminate) ||
                (m == modelEmbellish) ||
                (m == modelDepict)) {  // This will be a list soon...
              next_module = next_module->getRightExpander().module;
            } else {
              break;
            }
          }
          buffer->freshen_waveform = found_depict;
        }
        // TODO: add a process that eliminates very old RecordHead modules that
        // appear to be unused.
        // We are possibly not connected to them any more.
        // Or in some other way, remove them.
        // Or more likely, replace the record_heads list (removing an item from
        // a vector is not thread-safe).
      }

      // We may be being asked to wipe and/or reset. Both of these take more than one
      // process() call to complete.
      // If asked to do neither, we do neither, although we may be waiting for
      // a previous action to complete.
      // A wipe doesn't (currently) require us to watch the state progress, and it
      // doesn't prevent using the Buffer. So we don't particularly attend to it.
      // If only a reset is requested, then we start a reset. This can take while,
      // so we make sure not to allow another reset to interrupt it or another wipe
      // to occur while in progress.
      // If a reset AND a wipe are requested at the same process() call, only the reset
      // call is acted on.

      // Have we been asked to wipe?
      bool wipe_was_low = !wipe_trigger.isHigh();
      wipe_trigger.process(rescale(
          inputs[WIPE_TRIGGER_INPUT].getVoltage(), 0.1f, 2.0f, 0.0f, 1.0f));
      if (wipe_was_low && wipe_trigger.isHigh()) {
        // Flash the wpie light for a tenth of second.
        // Compute how many samples to show the light.
        wipe_light_countdown = std::floor(args.sampleRate / 10.0f);
      }
      bool reset = (params[RESET_BUTTON_PARAM].getValue() > 0.1f);
      // Note that we don't bother to set wipe_light_countdown when the user
      // presses the button; we just light up the button while it's
      // being pressed.
      bool wipe = (params[WIPE_BUTTON_PARAM].getValue() > 0.1f) ||
        (wipe_was_low && wipe_trigger.isHigh());

      if (reset) {
        if (buffer_initialized) {
          // Rerun the code at the top of process() on next call.
          buffer_initialized = false;
        }
      } else {
        if (wipe) {
          // TODO: Decide (or menu options) to pause all recording and playing
          // during a wipe? Or at least fade the players?
          buffer_change_worker->initiateWipe = true;
        }
      }
      // Set lights.
      lights[WIPE_BUTTON_LIGHT].setBrightness(
        wipe || wipe_light_countdown > 0 ? 1.0f : 0.0f);
      lights[RESET_BUTTON_LIGHT].setBrightness(reset ? 1.0f : 0.0f);
      lights[FILE_IO_LIGHT].setBrightness(
        prepare_worker && prepare_worker->busy ? 1.0f : 0.0f);
    }
  }

  std::string selectFolder() {
        std::string path_string = "";
        char *path = osdialog_file(
          OSDIALOG_OPEN_DIR,
          load_folder_name.empty() ? NULL : load_folder_name.c_str(),
          NULL, NULL);

        if (path != NULL) {
            path_string.assign(path);
            std::free(path);  // Required by osdialog_file().
        }

        return (path_string);
    }
};

struct MenuItemPickFolder : MenuItem
{
  Memory *module;

  void onAction(const event::Action &e) override
  {
    std::string filename = module->selectFolder();
    if (filename != "") {
      // We set this even if it's the same path as before. This gives the user a gesture to update
      // the list.
      PrepareTask* task = PrepareTask::LoadDirectoryTask(filename);
      if (!module->widget_module_queue.tasks.push(task)) {
        delete task;
      }
    }
  }
};


struct MemoryWidget : ModuleWidget {
  MemoryWidget(Memory* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Memory.svg"),
                         asset::plugin(pluginInstance, "res/Memory-dark.svg")));

    // Module is so narrow that we only include two screws instead of four.
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH,
                                           RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // WIPE button and trigger.
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(14.886, 14.817)),
                                             module, Memory::WIPE_BUTTON_PARAM,
                                             Memory::WIPE_BUTTON_LIGHT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.378, 14.817)), module,
                                             Memory::WIPE_TRIGGER_INPUT));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.16, 32.837)),
             module, Memory::SECONDS_PARAM));
    // RESET button.
    addParam(createLightParamCentered<VCVLightButton<
             MediumSimpleLight<WhiteLight>>>(mm2px(Vec(10.16, 46.959)),
                                             module, Memory::RESET_BUTTON_PARAM,
                                             Memory::RESET_BUTTON_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.378, 79.325)),
             module, Memory::TIPSY_LOAD_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.378, 95.795)),
             module, Memory::TIPSY_SAVE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(14.886, 79.325)),
             module, Memory::LOAD_TRIGGER_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(14.886, 95.795)),
             module, Memory::SAVE_TRIGGER_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(12.806, 112.537)),
             module, Memory::TIPSY_LOGGING_OUTPUT));

    // FILE I/O light.
   	addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(17.039, 121.986)),
             module, Memory::FILE_IO_LIGHT));
  }

  void appendContextMenu(Menu* menu) override {
    Memory* module = dynamic_cast<Memory*>(this->module);
      assert(module);
      menu->addChild(new MenuSeparator);
      menu->addChild(createMenuLabel("Pick Folder for Loading"));


      MenuItemPickFolder *menu_item_load_folder = new MenuItemPickFolder;
      if (module->load_folder_name.empty()) {
        menu_item_load_folder->text = "Click here to pick";  
      } else {
        menu_item_load_folder->text = module->load_folder_name;
      }
      menu_item_load_folder->module = module;
      menu->addChild(menu_item_load_folder);
      if (module->loadable_files.empty()) {
        menu->addChild(createMenuLabel("No .wav files seen in Loading directory"));
      } else {
        MenuItem* loadable_file_menu = createSubmenuItem("Load File", "",
          [=](Menu* menu) {
              for (const std::string& name : module->loadable_files) {
                menu->addChild(createCheckMenuItem(name, "",
                  [=]() {return name.compare(module->loaded_file) == 0;},
                  [=]() {
                    PrepareTask* task = PrepareTask::LoadFileTask(name, module->load_folder_name);
                    if (!module->widget_module_queue.tasks.push(task)) {
                      delete task;
                    }
                  }
                ));
              }
          }
        );
        menu->addChild(loadable_file_menu);
      }
      // TODO: pick saving folder.



  }
};

Model* modelMemory = createModel<Memory, MemoryWidget>("Memory");
