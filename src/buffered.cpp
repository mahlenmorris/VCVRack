#include <cstring>

#include "buffered.hpp"

std::shared_ptr<Buffer> findClosestMemory(Module* leftModule) {
  while (true) {
    if (!leftModule) return nullptr;
    if (IsMemoryEnsembleModel(leftModule->model)) {
      return dynamic_cast<BufferedModule*>(leftModule)->getHandle()->buffer;
    }
    auto m = leftModule->model;
    if (IsNonMemoryEnsembleModel(m)) {
      leftModule = leftModule->getLeftExpander().module;
    } else {
      return nullptr;
    }
  }
}

bool ModelHasColor(Model* model) {
  return ((model == modelRuminate) || (model == modelFixation) ||
          (model == modelEmbellish));
}

bool IsMemoryEnsembleModel(Model* model) {
  return ((model == modelMemory) || (model == modelMemoryCV));
}

bool IsNonMemoryEnsembleModel(Model* model) {
  return ((model == modelRuminate) || (model == modelFixation) ||
          (model == modelDepict) || (model == modelEmbellish) ||
          (model == modelBrainwash));
}

bool Buffer::IsValid() {
  return (length > 0) && (seconds > 0.0) && (left_array != nullptr) &&
         (right_array != nullptr);
}

int Buffer::NearHead(int position) {
  if (cv_rate) {
    // MemoryCV does not want fades and smoothing.
    return INT_MAX;
  }
  // TODO: not sure if correct when near the ends of the buffer.
  int nearest = INT_MAX;
  for (int i = 0; i < (int)record_heads.size(); ++i) {
    int other_position = record_heads[i].position;
    nearest = std::min(nearest, abs(other_position - position));
    nearest = std::min(nearest, abs(other_position + length - position));
    nearest = std::min(nearest, abs(other_position - (position + length)));
  }
  return nearest <= FADE_DISTANCE ? nearest : INT_MAX;
}

void Buffer::SetDirty(int position) {
  dirty[(int)floor(position / ((double)length / WAVEFORM_SIZE))] = true;
}

// Returns distance if near a recording head, except for the recording head
// with 'module_id', or INT_MAX if not considered "near".
// Typically called for the benefit of recording heads.
int Buffer::NearHeadButNotThisModule(int position, long long module_id) {
  if (cv_rate) {
    // MemoryCV does not want fades and smoothing.
    return INT_MAX;
  }

  // TODO: not sure if correct when near the ends of the buffer.
  int nearest = INT_MAX;
  for (int i = 0; i < (int)record_heads.size(); ++i) {
    if (record_heads[i].module_id == module_id) {
      continue;
    }
    int other_position = record_heads[i].position;
    nearest = std::min(nearest, abs(other_position - position));
    nearest = std::min(nearest, abs(other_position + length - position));
    nearest = std::min(nearest, abs(other_position - (position + length)));
  }
  return nearest <= FADE_DISTANCE ? nearest : INT_MAX;
}

void Buffer::Get(FloatPair* pair, double position) {
  if (!IsValid()) {
    for (int c = 0; c < PORT_MAX_CHANNELS; ++c) {
      pair->left[c] = 0.0f;
      pair->right[c] = 0.0f;
    }
    return;
  }
  assert(position >= 0.0);
  // When the buffer gets updated while Ruminate is playing, the position
  // can be way out of bounds. Don't crash then.
  if (position >= length) {
    position = 0.0;
  }
  if (cv_rate) {
    // MemoryCV does not do interpolation between samples!
    // (E.g., don't want intermediate values between two V/Oct values.)
    // Also need to scale down 'position', as length != true_length.
    int true_position = trunc(position / length * true_length);
    assert(true_position >= 0 && true_position < true_length);
    // The channels in a CV buffer are interleaved together.
    int base = true_position * PORT_MAX_CHANNELS;
    std::memcpy(pair->left, &left_array[base],
                PORT_MAX_CHANNELS * sizeof(float));
    std::memcpy(pair->right, &right_array[base],
                PORT_MAX_CHANNELS * sizeof(float));
  } else {
    // For a Memory with audio data, we want interpolation.
    int playback_start = trunc(position);
    int playback_end = trunc(playback_start + 1);
    if (playback_end >= length) {
      playback_end -= length;  // Should be zero.
    }
    float start_fraction = position - playback_start;
    pair->left[0] = left_array[playback_start] * (1.0 - start_fraction) +
                    left_array[playback_end] * (start_fraction);
    pair->right[0] = right_array[playback_start] * (1.0 - start_fraction) +
                     right_array[playback_end] * (start_fraction);
  }
}

void Buffer::GetDirect(FloatPair* pair, double position) {
  if (!IsValid()) {
    for (int c = 0; c < PORT_MAX_CHANNELS; ++c) {
      pair->left[c] = 0.0f;
      pair->right[c] = 0.0f;
    }
    return;
  }
  assert(position >= 0.0);
  assert(position < true_length);
  int pos = (int)position;
  if (cv_rate) {
    int base = pos * PORT_MAX_CHANNELS;
    std::memcpy(pair->left, &left_array[base],
                PORT_MAX_CHANNELS * sizeof(float));
    std::memcpy(pair->right, &right_array[base],
                PORT_MAX_CHANNELS * sizeof(float));
  } else {
    pair->left[0] = left_array[pos];
    pair->right[0] = right_array[pos];
  }
}

// Caller is responsible for only calling this when IsValid() is true.
void Buffer::Set(int position, const float* left, int left_chans,
                 const float* right, int right_chans, long long module_id) {
  if (!IsValid()) {
    return;
  }
  assert(position >= 0);
  assert(position < length);
  if (cv_rate) {
    // Because there are many positions for each CV position, we need to only
    // allow a write to occur on one position per CV position. I want it to be
    // the "center" of the CV position.
    int cv_position = trunc((double)position * true_length / length);
    int writable_position =
        trunc((double)(cv_position + 0.5) * length / true_length);
    if (position == writable_position) {
      int l_chans = std::min(std::max(left_chans, 0), PORT_MAX_CHANNELS);
      int r_chans = std::min(std::max(right_chans, 0), PORT_MAX_CHANNELS);
      left_channels = std::max(left_channels, std::max(1, l_chans));
      right_channels = std::max(right_channels, std::max(1, r_chans));
      int base = cv_position * PORT_MAX_CHANNELS;
      for (int c = 0; c < PORT_MAX_CHANNELS; ++c) {
        left_array[base + c] = (c < l_chans) ? left[c] : 0.0f;
        right_array[base + c] = (c < r_chans) ? right[c] : 0.0f;
      }
    }
  } else {
    left_array[position] = (left_chans > 0) ? left[0] : 0.0f;
    right_array[position] = (right_chans > 0) ? right[0] : 0.0f;
  }

  if (true_length < WAVEFORM_SIZE) {
    // In cases where there are very few samples compared to the WAVEFORM
    // resolution, the correct solution would be to SetDirty a range of the
    // waveform. But, no, that's really hard! We'll just recreate the whole
    // waveform.
    full_scan = true;
  } else {
    SetDirty(position);
  }

  // Update position for this Set() call's module.
  // Memory creates and updates this list during the module scan in
  // process().
  // If we don't find it now, we will when it updates.
  for (int i = 0; i < (int)record_heads.size(); ++i) {
    if (record_heads[i].module_id == module_id) {
      record_heads[i].position = position;
      record_heads[i].age = 0;
      break;
    }
  }
}

BufferTask::~BufferTask() {
  // Don't delete new_(left|right)_array, surely pointed to by something else.
}

BufferTask* BufferTask::SaveFileTask(FileOperationReporting* status,
                                     const std::string& file_path) {
  BufferTask* task = new BufferTask(SAVE_FILE);
  task->status = status;
  task->str1 = file_path;
  return task;
}

BufferTask* BufferTask::ReplaceTask(float* new_left, float* new_right,
                                    FileOperationReporting* status,
                                    int sample_count, double seconds,
                                    bool smooth_ends, int left_channels,
                                    int right_channels) {
  BufferTask* task = new BufferTask(REPLACE_AUDIO);
  task->new_left_array = new_left;
  task->new_right_array = new_right;
  task->status = status;
  task->sample_count = sample_count;
  task->seconds = seconds;
  task->smooth_endpoints = smooth_ends;
  task->left_channels = left_channels;
  task->right_channels = right_channels;
  return task;
}
