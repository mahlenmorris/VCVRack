#include <ctime>
#include <iomanip>
#include <thread>
#include <cstdlib> // for strtol

// VCV.
#include "plugin.hpp"
#include "osdialog.h"

// Mine.
#include "buffered.hpp"
#include "smoother.h"
#include "tipsy_utils.h"
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
// There are many threads in a Memory object which need to assign tasks to each other.
// They are:
// * The module itself (i.e., process()).
// * The UI Widget (events coming from the user).
// * The Prepare thread, which does long tasks like preparing audio files.
// * The BufferUpdate thread, for tasks that require changing the buffer. 

// These are the structs for doing that. Each struct corresponds to a single
// NoLockQueue, with a single writer and reader.
// All queues are owned by the module.

// PrepareTask's and BufferTask's (in buffered.h) get passed from thread to thread.
struct PrepareTask {
  enum Type {
    LOAD_DIRECTORY_SET,  // str1 is the directory name.
    LOAD_FILE,           // str1 is the selected file name, str2 is the directory name.
    // WIPE and RESET are really only different in how the length is set.
    MAKE_BLANK,          // seconds is length in seconds
    SAVE_FILE            // str1 is the full path and name.
  };
  Type type;
  double seconds;
  std::string str1, str2;
  float* new_left_array;
  float* new_right_array;
  int result_sample_count;
  std::vector<std::string>* loadable_files;
  FileOperationReporting* status;  // Owned by module.

  PrepareTask(const Type the_type) : type{the_type},
      new_left_array{nullptr}, new_right_array{nullptr},
      loadable_files{nullptr}, status{nullptr} {}

  ~PrepareTask() {
  }

  static PrepareTask* LoadFileTask(FileOperationReporting* reporting,
                                   const std::string& name,
                                   const std::string& directory) {
    PrepareTask* task = new PrepareTask(LOAD_FILE);
    task->status = reporting;
    task->str1 = name;
    task->str2 = directory;
    return task;
  }

  static PrepareTask* LoadDirectoryTask(const std::string& directory) {
    PrepareTask* task = new PrepareTask(LOAD_DIRECTORY_SET);
    task->str1 = directory;
    return task;
  }

  static PrepareTask* SaveFileTask(const std::string& file_path) {
    PrepareTask* task = new PrepareTask(SAVE_FILE);
    task->str1 = file_path;
    return task;
  }

  static PrepareTask* MakeBlank(double seconds) {
    PrepareTask* task = new PrepareTask(MAKE_BLANK);
    task->seconds = seconds;
    return task;
  }
};

struct WidgetToModuleQueue {
  // Only length 5 because, seriously, the UI can't really add tasks quickly.
  SpScLockFreeQueue<PrepareTask*, 5> tasks;
};

struct BufferToModuleQueue {
  // Only length 5 because there should be very few of these.
  SpScLockFreeQueue<PrepareTask*, 5> tasks;
};

struct ModuleToPrepareQueue {
  // Queue is limited to ten items, just so we cannot get absurdly far behind.
  SpScLockFreeQueue<PrepareTask*, 10> tasks;
};

struct PrepareToBufferQueue {
  // Queue is limited to ten items, just so we cannot get absurdly far behind.
  SpScLockFreeQueue<BufferTask*, 10> tasks;
};

struct ModuleToBufferQueue {
  // Queue is limited to five items, just so we cannot get absurdly far behind.
  SpScLockFreeQueue<BufferTask*, 5> tasks;
};

// Class devoted to handling things that replace or update the Buffer,
// apart from simple writes, or require that the buffer not be changing.
struct BufferChangeThread {
  BufferHandle* handle;
  PrepareToBufferQueue* prepare_buffer_queue;
  ModuleToBufferQueue* module_buffer_queue;
  BufferToModuleQueue* buffer_module_queue;
  std::vector<FileOperationReporting*>* reporters;
  // VCV's sample rate; the rate we expect process() to be called.
  float process_call_rate;
  // Sample rate for memory buffers and saved files.
  float memory_sample_rate;

  bool shutdown;
  // Indicator to UI that File I/O is happening.
  bool busy = false;

  BufferChangeThread(BufferHandle* the_handle, PrepareToBufferQueue* prepare_buffer_queue,
                     ModuleToBufferQueue* module_buffer_queue,
                     BufferToModuleQueue* buffer_module_queue,
                     std::vector<FileOperationReporting*>* reporters) :
      handle{the_handle}, prepare_buffer_queue{prepare_buffer_queue},
      module_buffer_queue{module_buffer_queue},
      buffer_module_queue{buffer_module_queue}, reporters{reporters},
      process_call_rate{0.0f}, memory_sample_rate{0.0f}, shutdown{false} {}

  void Halt() {
    shutdown = true;
  }

  void SetRates(float vcv_sample_rate, float memory_rate) {
    process_call_rate = vcv_sample_rate;
    memory_sample_rate = memory_rate;
  }

  // Updates the "waveforms" that Depict shows.
  void RefreshWaveform(std::shared_ptr<Buffer> buffer) {
    float sector_size = (double) buffer->true_length / WAVEFORM_SIZE;
    float peak_value = 0.0f;
    bool full_scan = buffer->full_scan;

    if (full_scan) {
      buffer->full_scan = false;  // I'm doing the scan now.
    }

    // For now, do this the most brute-force way; scan from bottom to top.
    for (int p = 0; !shutdown && p < WAVEFORM_SIZE; p++) {
      if (full_scan || buffer->dirty[p]) {
        float left_amplitude = 0.0, right_amplitude = 0.0;
        for (int i = (int) trunc(p * sector_size);
              !shutdown && i < std::min((int) trunc((p + 1) * sector_size), buffer->true_length);
              i++) {
          left_amplitude = std::max(left_amplitude,
                                    std::fabs(buffer->left_array[i]));
          right_amplitude = std::max(right_amplitude,
                                      std::fabs(buffer->right_array[i]));
        }
        buffer->waveform.points[p][0] = left_amplitude;
        buffer->waveform.points[p][1] = right_amplitude;

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

  void ReplaceAudio(std::shared_ptr<Buffer> buffer, BufferTask* task) {
    // Buffer is unavailable during this interval.
    buffer->length = 0;
    buffer->seconds = 0.0;
    // And mark every sector dirty.
    buffer->full_scan = true;

    if (buffer->left_array != nullptr) {
      // TODO: perhaps better to delay these deletions a bit?
      // Might help protect threads unaware that buffer is invalid
      // from accessing freed RAM.
      delete buffer->left_array;
    }
    buffer->left_array = task->new_left_array;
    if (buffer->right_array != nullptr) {
      delete buffer->right_array;
    }
    buffer->right_array = task->new_right_array;

    buffer->true_length = task->sample_count;
    if (buffer->cv_rate) {
      buffer->length = std::round(task->seconds * process_call_rate);
    } else {
      buffer->length = task->sample_count;
    }
    buffer->seconds = task->seconds;
    // Tell world we're done, IFF we have a status. WIPE's and RESET's don't.
    if (task->status != nullptr) {
      task->status->completed = LOAD_COMPLETED;
    }
    delete task;

    // WARN("length = %d", buffer->length);
    // WARN("true_length = %d", buffer->true_length);
    // WARN("seconds = %f", buffer->seconds);

    if (!buffer->cv_rate) {
      // There's no reason I can come up with to let there be a click between
      // the end and the beginning of the buffer in an *audio* file.
      // So if I suspect there will be one, add a Smooth to get rid of it.
      if (abs(buffer->left_array[0] - buffer->left_array[buffer->true_length - 1]) > 0.1 ||
          abs(buffer->right_array[0] - buffer->right_array[buffer->true_length - 1]) > 0.1) {
        Smooth* new_smooth = new Smooth(0, true);
          buffer->smooths.additions.push(new_smooth);
      }
    }
  }

  void Work() {
    // We deal with multiple work queues in this loop.
    while (!shutdown) {
      std::shared_ptr<Buffer> buffer = handle->buffer;
      if (buffer) {
        if (buffer->smooths.additions.size() > 0) {
          Smooth* item;
          while (buffer->smooths.additions.pop(item)) {
            // Check that creation_time is long enough ago that we're
            // confident that the new section is written.
            if (item->creation_time < 0 || (system::getTime() - item->creation_time > 0.001)) {
              smooth(buffer->left_array, buffer->right_array, item->position, 25, buffer->true_length);
              delete item;
            } else {
              buffer->smooths.additions.push(item);
              break;
            }
          }
        }

        if (module_buffer_queue->tasks.size() > 0) {
          BufferTask* task;
          while (module_buffer_queue->tasks.pop(task) && !shutdown) {
            switch (task->type) {
              case BufferTask::REPLACE_AUDIO: {
                WARN("There should not be a REPLACE_AUDIO task on the module_buffer_queue!");
                delete task;
              }
              break;
              case BufferTask::SAVE_FILE: {
                task->status->log_messages.lines.push(
                  "Starting to save '" + task->str1 + "'.");
                busy = true;
                AudioFile<float> audio_file;
                // Let's us collect the logs of any errors.
                audio_file.setLogQueue(&(task->status->log_messages));
                // This will tell AudioFile not use the internal buffer. Much faster to
                // not have to build a new buffer and copy to it, and less RAM-intensive.
                audio_file.setMemoryBuffer(buffer);

                // Do a fair bit of setup, so it can save properly.
                // Set both the number of channels and number of samples per channel
                audio_file.setAudioBufferSize(2, buffer->true_length);

                // Set the number of samples per channel
                audio_file.setNumSamplesPerChannel(buffer->true_length);

                // Set the number of channels. Memory is always stereo.
                audio_file.setNumChannels(2);
                audio_file.setBitDepth(32);  // This is what Audacity produces.
                audio_file.setSampleRate(memory_sample_rate);

                // SLOW: this call can take many seconds.
                // Wave file (implicit)
                bool worked = audio_file.save(task->str1);
                busy = false;
                if (worked) {
                  task->status->log_messages.lines.push(
                    "Completed save of '" + task->str1 + "'.");
                } else {
                  task->status->log_messages.lines.push(
                    "Unable to save '" + task->str1 + "'.");
                }               
                task->status->completed = SAVE_COMPLETED;
                delete task;

                // We may have added a file to the Load directory, so we tell the module
                // to rescan the load directory just in case.
                PrepareTask* rescan_task = PrepareTask::LoadDirectoryTask("");
                if (!buffer_module_queue->tasks.push(rescan_task)) {
                  delete rescan_task;
                }
              }
              break;
            }
          }
        } 

        if (prepare_buffer_queue->tasks.size() > 0) {
          BufferTask* task;
          while (prepare_buffer_queue->tasks.pop(task) && !shutdown) {
            switch (task->type) {
              case BufferTask::REPLACE_AUDIO: {
                ReplaceAudio(buffer, task);
              }
              break;
              case BufferTask::SAVE_FILE: {
                WARN("There should not be a SAVE_FILE task on the prepare_buffer_queue!");
                delete task;
              }
              break;
            }
          }
        } 

        if (buffer->replacements.queue.size() > 0) {
          BufferTask* task;
          while (buffer->replacements.queue.pop(task) && !shutdown) {
            switch (task->type) {
              case BufferTask::REPLACE_AUDIO: {
                //WARN("Sending REPLACE_AUDIO task from replacements queue...");
                //WARN("receiver says replace_task->sample_count = %d", task->sample_count);
                // Now that we are within Memory, we can add a LOG 
                FileOperationReporting* reporter = new FileOperationReporting();
                task->status = reporter;
                // Sooo, technically this adds a second writer to this structure that is only thread safe with one writer.
                reporters->push_back(reporter);
                reporter->log_messages.lines.push(
                  "Brainwash replaced Memory with a new recording that is " +
                  std::to_string(task->seconds) + " seconds long."
                );
                ReplaceAudio(buffer, task);
                reporter->completed = LOAD_COMPLETED;
              }
              break;
              case BufferTask::SAVE_FILE: {
                WARN("There should not be a SAVE_FILE task on the 'replacements' queue!");
                delete task;
              }
              break;
            }
          }
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
};  // BufferChangeThread.

// Class devoted to handling the very lengthy (compared to single sample)
// process of dealing with file systems.
// Much of the logic is directly lifted from voxglitch's modules.
// All mistakes are mine.
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

    for (auto path : dirList)  {
      if ((rack::string::lowercase(system::getExtension(path)) == "wav") ||
          (rack::string::lowercase(system::getExtension(path)) == ".wav")) {
        loadable_files->push_back(system::getFilename(path));
      }
    }
    busy = false;
  }

  void ConvertFileToSamples(PrepareTask* task, const AudioFile<float>& audio_file) {
    assert(task->type == PrepareTask::LOAD_FILE);
    // One goal is to minimize the amount of downtime for the buffer,
    // since the AudioFile -> Buffer process is time consuming for large files.
    // So the conversion happens before we swap in the memory.
    int samples = std::round(audio_file.getLengthInSeconds() * sample_rate);
    int file_samples = audio_file.getNumSamplesPerChannel();
    float* new_left_array = new float[samples];
    float* new_right_array = new float[samples];

    // Now fill from the audio_file. Transforms we need to apply are:
    // * Sample rate may be different from the file and VCV.
    // * We typically range from -10.0 to 10.0, AudioFile ranges from -1.0 to 1.0.
    // * File might have been in mono. The norm seems to be to put a mono signal 
    //   on both channels.
    bool file_is_mono = audio_file.getNumChannels() == 1;
    // TODO: put in an optimization if sample rates are the same. Less math to do.
    double sample_rate_ratio = 1.0 * audio_file.getSampleRate() / sample_rate;
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
       (audio_file.samples[0][playback_start] * (1.0 - start_fraction) +
        audio_file.samples[0][playback_end] * (start_fraction));
      if (file_is_mono) {
        new_right_array[i] = new_left_array[i];
      } else {
        new_right_array[i] = 10.0 * 
         (audio_file.samples[1][playback_start] * (1.0 - start_fraction) +
          audio_file.samples[1][playback_end] * (start_fraction));
      }
    }
    task->new_left_array = new_left_array;
    task->new_right_array = new_right_array;
    task->result_sample_count = samples;
  }  

  void Work() {
    while (!shutdown) {
      if (module_file_queue->tasks.size() > 0) {
        PrepareTask* task;
        while (module_file_queue->tasks.pop(task) && !shutdown) {
          switch (task->type) {
            case PrepareTask::LOAD_FILE: {
              task->status->log_messages.lines.push(
                "Starting to read '" + task->str1 + "'.");
              busy = true;
              AudioFile<float> audio_file;
              // Let's us collect the logs of any errors.
              audio_file.setLogQueue(&(task->status->log_messages));
              // SLOW: this call can take many seconds.
              bool worked = audio_file.load(system::join(task->str2, task->str1));
              busy = false;
              // WARN("Completed load of %s", task->str1.c_str());             
              // WARN("samples = %d, seconds = %f", audio_file.getNumSamplesPerChannel(), audio_file.getLengthInSeconds());
              // WARN("sample rate = %d", audio_file.getSampleRate());
              // WARN("bit depth = %d", audio_file.getBitDepth());

              // If worked, need to fill buffer.
              if (worked) {
                task->status->log_messages.lines.push(
                  "Completed read of '" + task->str1 + "'.");
                task->status->log_messages.lines.push(
                  "It is " + std::to_string(audio_file.getLengthInSeconds()) +
                  " seconds long.");
                
                // This part can also be slow, since it walks over every sample.
                busy = true;
                ConvertFileToSamples(task, audio_file);
                busy = false;
                double seconds = audio_file.getLengthInSeconds();
                // Done with the audio_file.

                // Send task to BufferChangeThread.
                BufferTask* replace_task = BufferTask::ReplaceTask(
                  task->new_left_array, task->new_right_array, task->status,
                  task->result_sample_count, seconds);  
                if (!prepare_buffer_queue->tasks.push(replace_task)) {
                  delete task->new_left_array;
                  delete task->new_right_array;
                  delete replace_task;  // Queue is full, shed load.
                }
              } else {
                task->status->log_messages.lines.push(
                  "Failed to read '" + task->str1 + "'.");
                task->status->completed = LOAD_COMPLETED;
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

            case PrepareTask::MAKE_BLANK: {
              assert(sample_rate > 1.0);
              int sample_count = std::round(task->seconds * sample_rate);
              float* new_left_array = new float[sample_count];
              float* new_right_array = new float[sample_count];

              for (int i = 0; i < sample_count && !shutdown; ++i) {
                new_left_array[i] = 0.0f;
                new_right_array[i] = 0.0f;
              }

              // Send task to BufferChangeThread.
              BufferTask* replace_task = BufferTask::ReplaceTask(
                new_left_array, new_right_array, nullptr, sample_count, task->seconds);  
              if (!prepare_buffer_queue->tasks.push(replace_task)) {
                delete replace_task;  // Queue is full, shed load.
              }
              delete task;
            }
            break;
            case PrepareTask::SAVE_FILE: {
              WARN("There should not be a SAVE_FILE task on the module_file_queue!");
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
};  // PrepareThread.

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

  // Initialization happens on startup.
  bool buffer_initialized = false;
  bool init_in_progress = false;

  // Threads and workers for doing long tasks.
  BufferChangeThread* buffer_change_worker;
  std::thread* buffer_change_thread;
  // For tasks that don't directly change the Buffer, but might send changes to the
  // buffer_change_thread. Especially important for changes that take long periods of time.
  PrepareThread* prepare_worker;
  std::thread* prepare_thread;

  // lock-free queues for coordinating actions between the threads.
  WidgetToModuleQueue widget_module_queue;
  ModuleToPrepareQueue module_prepare_queue;
  ModuleToBufferQueue module_buffer_queue;
  PrepareToBufferQueue prepare_buffer_queue;
  BufferToModuleQueue buffer_module_queue;

  // For wiping contents.
  dsp::SchmittTrigger wipe_trigger;
  bool wipe_button_pressed = false;

   // Keeps lights on buttons lit long enough to see.
  int wipe_light_countdown = 0;
  int reset_light_countdown = 0;

  // Make sure we only reset once when RESET button is pressed.
  bool reset_button_pressed = false;
  
  // For Loading and Saving contents.
  std::string load_folder_name;  // For menu to display.
  std::vector<std::string> loadable_files;  // List found in menu.
  std::string loaded_file;  // Checked item in menu.
  std::string save_folder_name;  // For menu to display.
  // Menu option to load "loaded_file" on patch start.
  bool load_latest_file_on_start;
  // Trigger to actually initiate that startup_load.
  bool initiate_startup_load;

  std::vector<FileOperationReporting*> reporters;
  // Makes the output trigger for loads and saves when needed.
  dsp::PulseGenerator load_generator;
  dsp::PulseGenerator save_generator;

  // Listening to the Tipsy connection for loading/saving files.
  unsigned char load_recv_buffer[recvBufferSize];
  tipsy::ProtocolDecoder load_decoder;
  unsigned char save_recv_buffer[recvBufferSize];
  tipsy::ProtocolDecoder save_decoder;

  // Mechanism for sending text out.
  TextSender log_message_sender;

  // We sweep the connected modules every NN samples. Some UI-related tasks are 
  // not as latency-sensitive as the audio thread, and we don't need to do often.
  int assign_color_countdown = 0;

  // Memory and MemoryCV differ only slightly, and most of the difference is how Buffer behaves.
  // This flag tells the code which to behave like.
  bool cv_rate;


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

    buffer_change_worker = new BufferChangeThread(getHandle(), &prepare_buffer_queue,
                                                  &module_buffer_queue, &buffer_module_queue,
                                                  &reporters);
    buffer_change_thread = new std::thread(&BufferChangeThread::Work, buffer_change_worker);
    prepare_worker = new PrepareThread(&module_prepare_queue, &prepare_buffer_queue);
    prepare_thread = new std::thread(&PrepareThread::Work, prepare_worker);

    load_decoder.provideDataBuffer(load_recv_buffer, recvBufferSize);
    save_decoder.provideDataBuffer(save_recv_buffer, recvBufferSize);

    load_latest_file_on_start = false;
    initiate_startup_load = false;
  }

  ~Memory() {
    // Stop prepare_worker first, since it sends tasks to buffer_change.
    if (prepare_worker != nullptr) {
      prepare_worker->Halt();
      if (prepare_thread != nullptr) {
        prepare_thread->join();
        delete prepare_thread;
      }
      delete prepare_worker;
    }
    (*getHandle()).buffer.reset();
    if (buffer_change_worker != nullptr) {
      buffer_change_worker->Halt();
      if (buffer_change_thread != nullptr) {
        buffer_change_thread->join();
        delete buffer_change_thread;
      }
      delete buffer_change_worker;
    }
  }

  // Save and retrieve menu choice(s).
  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    if (!load_folder_name.empty()) {
      json_object_set_new(rootJ, "load_folder", json_string(load_folder_name.c_str()));
    }
    if (!save_folder_name.empty()) {
      json_object_set_new(rootJ, "save_folder", json_string(save_folder_name.c_str()));
    }
    json_object_set_new(rootJ, "load_latest_on_start",
                        json_integer(load_latest_file_on_start ? 1 : 0));
    if (!loaded_file.empty()) {
      json_object_set_new(rootJ, "loaded_file", json_string(loaded_file.c_str()));
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
    json_t* save_folderJ = json_object_get(rootJ, "save_folder");
    if (save_folderJ) {
      save_folder_name.assign(json_string_value(save_folderJ));
    }
    json_t* loaded_fileJ = json_object_get(rootJ, "loaded_file");
    if (loaded_fileJ) {
      loaded_file.assign(json_string_value(loaded_fileJ));
    }
    json_t* load_latestJ = json_object_get(rootJ, "load_latest_on_start");
    if (load_latestJ) {
      load_latest_file_on_start = json_integer_value(load_latestJ) > 0;
      // Set load in motion. process() will actually kick it off.
      if (load_latest_file_on_start) {
        initiate_startup_load = true;
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

  // Both process() and processBypass() call this.
  void HandleLights(const ProcessArgs& args) {
    // Periodically assign colors to each connected module's light.
    // We also take this opportunity to add any unknown recording heads to
    // the buffer->record_heads vector.
    // I'd considered putting these tasks on a background thread, but
    // getRightExpander() is a method on Module.
    // And we can't put this in a call to onExpanderChange(), because events
    // several modules down can affect these results.
    if (--assign_color_countdown <= 0) {
      // One sixtieth of a second.
      assign_color_countdown = (int) (args.sampleRate / 60);

      std::shared_ptr<Buffer> buffer = getHandle()->buffer;
      if (buffer) {  // Checks for null.
        Module* next_module = getRightExpander().module;
        int color_index = -1;
        int distance = 0;
        bool found_depict = false;
        while (next_module) {
          if (ModelHasColor(next_module->model)) {
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
          if (IsNonMemoryEnsembleModel(m)) {  // This will be a list soon...
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
  }

  // Yes, it's little weird to bypass a Memory, but, eh, if you want it to stop loading
  // new files based on Tipsy input...sure, people might do it.
  void processBypass(const ProcessArgs& args) override {
    HandleLights(args);  // It really looks like a bug if the lights don't get assigned right.
  }

  void process(const ProcessArgs& args) override {
    // This is just for the initial creation of the buffer.
    // We can't fill the buffer until process() is called, since we don't know
    // what the sample rate is.
    if (!buffer_initialized) {
      // We don't pay any attention to the buttons, etc, until the buffer is initialized.
      if (!init_in_progress) {
        // Confirm that we can read the sample rate before starting a fill.
        // Sometimes during startup, sampleRate is still zero.
        float sample_rate = args.sampleRate;
        if (sample_rate > 1.0) {
          buffer_change_worker->SetRates(sample_rate, cv_rate ? CV_SAMPLE_RATE : sample_rate);
          prepare_worker->SetRate(cv_rate ? CV_SAMPLE_RATE : sample_rate);
          PrepareTask* task = PrepareTask::MakeBlank(params[SECONDS_PARAM].getValue());
          if (!module_prepare_queue.tasks.push(task)) {
            delete task;
          }
          init_in_progress = true;
        }
      } else {
        std::shared_ptr<Buffer> buffer = getHandle()->buffer;
        if (buffer && buffer->IsValid()) {
          // Filling + wiping done.
          init_in_progress = false;
          buffer_initialized = true;  // Now we will never look at this section again.
        }
      }
    } else {
      // This is the post-initialization part of process().
      // We may be asked to load a .wav file on startup. We wait until the buffer is initialized before
      // attempting this, however, since the operation might fail, and I'd rather have *some* buffer than none.
      if (initiate_startup_load && !load_folder_name.empty() && !loaded_file.empty()) {
        initiate_startup_load = false;
        FileOperationReporting* reporter = new FileOperationReporting();
        reporters.push_back(reporter);
        PrepareTask* task = PrepareTask::LoadFileTask(
          reporter, loaded_file, load_folder_name);
        if (!widget_module_queue.tasks.push(task)) {
          std::string message = "ERROR: Queue is full, cannot load '" +
            loaded_file + "' from '" + load_folder_name + "'.";
          reporter->log_messages.lines.push(message);
          reporter->completed = LOAD_COMPLETED;
          delete task;
        }
      }

      // Some lights are lit by triggers or button presses; these enable them to be
      // lit long enough to be seen by humans.
      if (wipe_light_countdown > 0) {
        wipe_light_countdown--;
      }
      if (reset_light_countdown > 0) {
        reset_light_countdown--;
      }

      // Process text coming in via Tipsy ports.
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
                std::string message = "ERROR: no number found in '" + next + "', expecting " +
                  "a string like '#3'.";
                log_message_sender.AddToQueue(message);
              } else {
                char* endptr;
                long parsed_long = strtol(next.c_str() + start_pos, &endptr, 10);

                // Check for conversion errors
                if (endptr == next.c_str() + start_pos || *endptr != '\0') {
                  // Handle the case where the conversion failed
                  std::string message = "ERROR: couldn't understand '" + next + "', expecting " +
                    "a string like '#3'.";
                  log_message_sender.AddToQueue(message);
                } else if (parsed_long > INT_MAX || parsed_long < INT_MIN) {
                  // TODO: is this condition even possible?
                  // Handle overflow if the parsed value is outside int range
                  std::string message = "ERROR: the number found in '" + next + "' is too large, " +
                    "expecting a string like '#3'.";
                  log_message_sender.AddToQueue(message);
                } else {
                  int parsed_int = static_cast<int>(parsed_long); // Cast to int if successful
                  // Now pick the relevant file in the directory.
                  // First make sure in the range.
                  // Yes, we're zero-indexed for reading this list.
                  parsed_int = parsed_int % loadable_files.size();

                  // OK, so this isn't the precisely correct queue, but we check it just below,
                  // so more clean to do this.
                  FileOperationReporting* reporter = new FileOperationReporting();
                  reporters.push_back(reporter);
                  PrepareTask* task = PrepareTask::LoadFileTask(
                    reporter, loadable_files[parsed_int], load_folder_name);
                  if (!widget_module_queue.tasks.push(task)) {
                    std::string message = "ERROR: Queue is full, cannot load '" +
                      loadable_files[parsed_int] + "' (which you asked for with '" +
                      next + "'.";
                    reporter->log_messages.lines.push(message);
                    reporter->completed = LOAD_COMPLETED;
                    delete task;
                  }
                }
              }
            } else {
              // Not a number, just a name.
              // No, this isn't the precisely correct queue, but we check it just below,
              // so simpler to do this.
              FileOperationReporting* reporter = new FileOperationReporting();
              reporters.push_back(reporter);
              PrepareTask* task = PrepareTask::LoadFileTask(
                reporter, next, load_folder_name);
              if (!widget_module_queue.tasks.push(task)) {
                std::string message = "ERROR: Queue is full, cannot load '" +
                  next + "'.";
                reporter->log_messages.lines.push(message);
                reporter->completed = LOAD_COMPLETED;
                delete task;
              }
            }
          }
        }
      }
      // The SAVE file case is simpler, since there's no #NNN syntax.
      // TODO: should there be a text gesture that saves using the default name? Not certain
      // I see that being useful.
      if (inputs[TIPSY_SAVE_INPUT].isConnected()) {
        auto decoder_status = save_decoder.readFloat(
            inputs[TIPSY_SAVE_INPUT].getVoltage());
        if (!save_decoder.isError(decoder_status) &&
            decoder_status == tipsy::DecoderResult::BODY_READY &&
            std::strcmp(save_decoder.getMimeType(), "text/plain") == 0) {
          std::string next(std::string((const char *) save_recv_buffer));
          if (next.size() > 0) {
            FileOperationReporting* reporter = new FileOperationReporting();
            reporters.push_back(reporter);

            BufferTask* task = BufferTask::SaveFileTask(
              reporter, system::join(save_folder_name, next));
            if (!module_buffer_queue.tasks.push(task)) {
              std::string message = "ERROR: Queue is full, cannot save '" +
                next + "'.";
              reporter->log_messages.lines.push(message);
              reporter->completed = SAVE_COMPLETED;
              delete task;
            }
          }
        }
      }

      // Deal with tasks from the Buffer thread.
      if (buffer_module_queue.tasks.size() > 0) {
        PrepareTask* task;
        while (buffer_module_queue.tasks.pop(task)) {
          switch (task->type) {
            case PrepareTask::LOAD_FILE: {
              WARN("There should not be a LOAD_FILE task on the buffer_module_queue!");
              delete task;
            }
            break;

            case PrepareTask::SAVE_FILE: {
              WARN("There should not be a SAVE_FILE task on the buffer_module_queue!");
              delete task;
            }
            break;

            case PrepareTask::LOAD_DIRECTORY_SET: {
              if (!load_folder_name.empty()) {
                task->str1 = load_folder_name;
                task->loadable_files = &loadable_files;
                if (!module_prepare_queue.tasks.push(task)) {
                  delete task;
                }
              } else {
                delete task;
              }
            }
            break;

            case PrepareTask::MAKE_BLANK: {
              WARN("There should not be a MAKE_BLANK task on the buffer_module_queue!");
              delete task;
            }
            break;
          }
        }
      } 

      // Deal with tasks from the Widget (aka, the menu) and from the Tipsy inputs.
      if (widget_module_queue.tasks.size() > 0) {
        PrepareTask* task;
        while (widget_module_queue.tasks.pop(task)) {
          switch (task->type) {
            case PrepareTask::LOAD_FILE: {
              // If this came from menu, it needs a reporter.
              if (task->status == nullptr) {
                FileOperationReporting* reporter = new FileOperationReporting();
                reporters.push_back(reporter);
                task->status = reporter;
              }
              // TODO: it'd be nice if we could actually interrupt a file load with a new load,
              // as the Tipsy automation means we could receive load requests far faster than we
              // can process them. Best to not get stuck waiting for a really slow network file
              // to load.
              if (!module_prepare_queue.tasks.push(task)) {
                std::string message = "ERROR: Queue is full, cannot load '" +
                  task->str1 + "'.";
                task->status->log_messages.lines.push(message);
                task->status->completed = LOAD_COMPLETED;
                delete task;
              } else {
                // This conception of "loaded_file" makes less sense.
                // TODO: fix this.
                loaded_file = task->str1;
              }
            }
            break;

            case PrepareTask::SAVE_FILE: {
              // Reformat this task to send it to the BufferChangeThread.
              // Because we want to ensure that the buffer does NOT get exchanged while we are saving it.
              FileOperationReporting* reporter = task->status;
              // If this came from menu, it needs a reporter.
              if (reporter == nullptr) {
                reporter = new FileOperationReporting();
                reporters.push_back(reporter);
              }
              BufferTask* save_task = BufferTask::SaveFileTask(reporter, task->str1);
              delete task;
              if (!module_buffer_queue.tasks.push(save_task)) {
                std::string message = "ERROR: Queue is full, cannot save '" +
                  save_task->str1 + "'.";
                save_task->status->log_messages.lines.push(message);
                save_task->status->completed = SAVE_COMPLETED;
                delete save_task;
              }
            }
            break;

            case PrepareTask::LOAD_DIRECTORY_SET: {
              load_folder_name = task->str1;
              task->loadable_files = &loadable_files;
              if (!module_prepare_queue.tasks.push(task)) {
                delete task;
              }
            }
            break;

            case PrepareTask::MAKE_BLANK: {
              WARN("There should not be a MAKE_BLANK task on the widget_module_queue!");
              delete task;
            }
            break;
          }
        }
      } 

      HandleLights(args);

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

      bool wipe_clicked = false;
      if ((params[WIPE_BUTTON_PARAM].getValue() > 0.1f)) {
        if (!wipe_button_pressed) { 
          wipe_button_pressed = true;
          wipe_clicked = true;
        }
      } else {
        wipe_button_pressed = false;
      }
      bool wipe = wipe_clicked || (wipe_was_low && wipe_trigger.isHigh());
      if (wipe) {
        // Flash the wipe light for a tenth of second.
        // Compute how many samples to show the light.
        wipe_light_countdown = std::floor(args.sampleRate / 10.0f);
      }

      bool reset = false;
      if ((params[RESET_BUTTON_PARAM].getValue() > 0.1f)) {
        if (!reset_button_pressed) { 
          reset_button_pressed = true;
          reset = true;
          reset_light_countdown = std::floor(args.sampleRate / 10.0f);
        }
      } else {
        reset_button_pressed = false;
      }

      // RESET takes precedence over a WIPE.
      if (reset && args.sampleRate > 1.0) {
        PrepareTask* task = PrepareTask::MakeBlank(params[SECONDS_PARAM].getValue());
        if (!module_prepare_queue.tasks.push(task)) {
          delete task;
        }
      } else {
        if (wipe) {
          std::shared_ptr<Buffer> buffer = getHandle()->buffer;
          if (buffer) {  // It should be, but let's be certain.
            PrepareTask* task = PrepareTask::MakeBlank(buffer->seconds);
            if (!module_prepare_queue.tasks.push(task)) {
              delete task;
            }
          }
        }
      }

      // Attend to any reporters, if need be.
      for (int i = 0; i < (int) reporters.size(); ++i) {
        FileOperationReporting* reporter = reporters[i];
        // Add all lines in the report to the log here.
        if (reporter->log_messages.lines.size() > 0) {
          std::string line;
          while (reporter->log_messages.lines.pop(line)) {
            log_message_sender.AddToQueue(line);  // Need to add a "/n"? No.
          }
        }
        if (reporter->completed > IN_PROGRESS) {
          if (reporter->completed == LOAD_COMPLETED) {
            load_generator.trigger(1e-3f);
          } else if (reporter->completed == SAVE_COMPLETED) {
            save_generator.trigger(1e-3f);
          }
          delete reporter;
          reporters.erase(reporters.begin() + i);
          --i;  // Go back one, since the ith element has changed.
        }
      }

      // Set the values for the output triggers.
      outputs[LOAD_TRIGGER_OUTPUT].setVoltage(
        load_generator.process(args.sampleTime) ? 10.0f : 0.0f);
      outputs[SAVE_TRIGGER_OUTPUT].setVoltage(
        save_generator.process(args.sampleTime) ? 10.0f : 0.0f);

      // Output next value for the log.
      log_message_sender.ProcessEncoder(TIPSY_LOGGING_OUTPUT, &outputs);

      // Set lights.
      lights[WIPE_BUTTON_LIGHT].setBrightness(
        wipe || wipe_light_countdown > 0 ? 1.0f : 0.0f);
      lights[RESET_BUTTON_LIGHT].setBrightness(reset_light_countdown ? 1.0f : 0.0f);
      lights[FILE_IO_LIGHT].setBrightness(
        (prepare_worker && prepare_worker->busy) ||
        (buffer_change_worker && buffer_change_worker->busy) ? 1.0f : 0.0f);
    }
  }

  std::string selectLoadFolder() {
    std::string path_string = "";
    char *path = osdialog_file(
      OSDIALOG_OPEN_DIR,
      load_folder_name.empty() ?
        (save_folder_name.empty() ? NULL : save_folder_name.c_str()) :
        load_folder_name.c_str(),
      NULL, NULL);

    if (path != NULL) {
        path_string.assign(path);
        std::free(path);  // Required by osdialog_file().
    }

    return (path_string);
  }

  std::string selectSaveFolder() {
    std::string path_string = "";
    char *path = osdialog_file(
      OSDIALOG_OPEN_DIR,
      save_folder_name.empty() ?
        (load_folder_name.empty() ? NULL : load_folder_name.c_str()) :
        save_folder_name.c_str(),
      NULL, NULL);

    if (path != NULL) {
        path_string.assign(path);
        std::free(path);  // Required by osdialog_file().
    }

    return (path_string);
  }

  std::string selectSaveFile() {
    std::string path_string = "";
     // I like things that default file names to a time stamp. 
    char date_buffer[80];

    // Convert unix time to local time structure.
    time_t unixTime = static_cast<time_t>(system::getUnixTime());
    struct tm* localTime = localtime(&unixTime);
    strftime(date_buffer, sizeof(date_buffer), "Memory %Y-%m-%d %H-%M-%S.wav", localTime);

    // Default to save folder. If not set, default to load_folder.
    char *path = osdialog_file(
      OSDIALOG_SAVE,
      save_folder_name.empty() ?
        (load_folder_name.empty() ? NULL : load_folder_name.c_str()) :
        save_folder_name.c_str(),
      date_buffer, osdialog_filters_parse("Wav:wav"));

    if (path != NULL) {
        path_string.assign(path);
        std::free(path);  // Required by osdialog_file().
    }
    return (path_string);
  }
};

struct MenuItemPickLoadFolder : MenuItem {
  Memory *module;

  void onAction(const event::Action &e) override {
    std::string filename = module->selectLoadFolder();
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

struct MenuItemPickSaveFolder : MenuItem {
  Memory *module;

  void onAction(const event::Action &e) override {
    std::string filename = module->selectSaveFolder();
    if (filename != "") {
      module->save_folder_name = filename;
    }
  }
};

struct MenuItemPickSaveFile : MenuItem {
  Memory *module;

  void onAction(const event::Action &e) override {
    std::string file_path = module->selectSaveFile();
    if (file_path != "") {
      // We set this even if it's the same path as before. This gives the user a gesture to update
      // the list.
      PrepareTask* task = PrepareTask::SaveFileTask(file_path);
      if (!module->widget_module_queue.tasks.push(task)) {
        delete task;
      }
    }
  }
};


struct MemoryWidget : ModuleWidget {
  MemoryWidget(Memory* module) {
    setModule(module);
    this->SetPanels();  // this-> forces the call to the overidden method.

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

  virtual void SetPanels() {
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Memory.svg"),
                         asset::plugin(pluginInstance, "res/Memory-dark.svg")));
  }

  void appendContextMenu(Menu* menu) override {
    Memory* module = dynamic_cast<Memory*>(this->module);
    assert(module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("Pick Folder for Loading"));

    MenuItemPickLoadFolder* menu_item_load_folder = new MenuItemPickLoadFolder;
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
                  PrepareTask* task = PrepareTask::LoadFileTask(nullptr,
                    name, module->load_folder_name);
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
    menu->addChild(createBoolPtrMenuItem("Load most recent file on module start", "",
                                         &(module->load_latest_file_on_start)));
    // TODO: Add menu action to just load some random file without setting the load folder.

    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("Pick Folder for Saving"));

    MenuItemPickSaveFolder* menu_item_save_folder = new MenuItemPickSaveFolder;
    if (module->save_folder_name.empty()) {
      menu_item_save_folder->text = "Click here to pick";  
    } else {
      menu_item_save_folder->text = module->save_folder_name;
    }
    menu_item_save_folder->module = module;
    menu->addChild(menu_item_save_folder);
    MenuItemPickSaveFile* menu_item_save_file = new MenuItemPickSaveFile;
    menu_item_save_file->text = "Save to File...";  
    menu_item_save_file->module = module;
    menu->addChild(menu_item_save_file);

    // Be a little clearer how to make this module do anything.
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel(
      "Put any of these modules directly to my right: Brainwash, Depict, Embellish, "));
    menu->addChild(createMenuLabel(
      "Fixation, and Ruminate. See my User Manual for details and usage videos."));
  }
};

struct MemoryCV : Memory {
  MemoryCV() : Memory() {
    cv_rate = true;
    std::shared_ptr<Buffer> buffer = getHandle()->buffer;
    if (buffer) {  // Pretty convinced this is always the case.
      buffer->cv_rate = true;
    } else {
      WARN("MemoryCV(): buffer is null!");
    }
  }
};

struct MemoryCVWidget : MemoryWidget {
  MemoryCVWidget(MemoryCV* module) : MemoryWidget((Memory*) module) {
    SetPanels();
  }

  void SetPanels() override {
    setPanel(createPanel(asset::plugin(pluginInstance, "res/MemoryCV.svg"),
                         asset::plugin(pluginInstance, "res/MemoryCV-dark.svg")));
  }
};

Model* modelMemory = createModel<Memory, MemoryWidget>("Memory");

Model* modelMemoryCV = createModel<MemoryCV, MemoryCVWidget>("MemoryCV");

