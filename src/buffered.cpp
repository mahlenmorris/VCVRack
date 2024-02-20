#include "buffered.hpp"

Buffer* findClosestMemory(Module* leftModule) {
  while (true) {
    if (!leftModule) return nullptr;
    if (leftModule->model == modelMemory) {
      return dynamic_cast<BufferedModule*>(leftModule)->getBuffer();
    }
    auto m = leftModule->model;
    if ((m == modelRecall) ||
        (m == modelRemember) ||
        (m == modelDisplay)) {  // This will be a list soon...
      leftModule = leftModule->getLeftExpander().module;
    } else {
      return nullptr;
    }
  }
}

bool Buffer::IsValid() {
  return (length > 0) && (seconds > 0.0) && (left_array != nullptr) &&
         (right_array != nullptr);
}

bool Buffer::NearHead(int position) {
  // TODO: not sure if correct when near the ends of the buffer.
  for (int i = 0; i < (int) record_heads.size(); ++i) {
    if (abs(record_heads[i].position - position) < 60) {
      // WARN("head = %d, pos = %d", record_heads[i].position, position);
      return true;
    }
    if (abs(record_heads[i].position + length - position) < 60) {
      // WARN("head = %d, pos = %d", record_heads[i].position, position);
      return true;
    }
    if (abs(record_heads[i].position - (position + length)) < 60) {
      // WARN("head = %d, pos = %d", record_heads[i].position, position);
      return true;
    }
  }
  return false;
}

void Buffer::SetDirty(int position) {
  dirty[(int) floor(position / ((double) length / WAVEFORM_SIZE))] = true;
}

// Returns true if near a recording head, except for module_id.
// Typically called for the benefit of play heads that are part of recording
// heads.
bool Buffer::NearHeadButNotThisModule(int position, long long module_id) {
  // TODO: not sure if correct when near the ends of the buffer.
  for (int i = 0; i < (int) record_heads.size(); ++i) {
    if (record_heads[i].module_id == module_id) {
      continue;
    }
    if (abs(record_heads[i].position - position) < 60) {
      // WARN("head = %d, pos = %d", record_heads[i].position, position);
      return true;
    }
    if (abs(record_heads[i].position + length - position) < 60) {
      // WARN("head = %d, pos = %d", record_heads[i].position, position);
      return true;
    }
    if (abs(record_heads[i].position - (position + length)) < 60) {
      // WARN("head = %d, pos = %d", record_heads[i].position, position);
      return true;
    }
  }
  return false;
}

void Buffer::Get(FloatPair *pair, double position) {
  assert(position >= 0.0);
  assert(position < length);
  int playback_start = trunc(position);
  int playback_end = trunc(playback_start + 1);
  if (playback_end >= length) {
    playback_end -= length;  // Should be zero.
  }
  float start_fraction = position - playback_start;
  pair->left = left_array[playback_start] * (1.0 - start_fraction) +
                left_array[playback_end] * (start_fraction);
  pair->right = right_array[playback_start] * (1.0 - start_fraction) +
                right_array[playback_end] * (start_fraction);
}

// Caller is responsible for only calling this when IsValid() is true.
void Buffer::Set(int position, float left, float right, long long module_id) {
  assert(position >= 0);
  assert(position < length);
  left_array[position] = left;
  right_array[position] = right;
  SetDirty(position);

  // Update (or create) a trace for this Set() call.
  // TODO: Memory should create/update this list during the module scan in
  // process().
  bool found = false;
  for (int i = 0; i < (int) record_heads.size(); ++i) {
    if (record_heads[i].module_id == module_id) {
      found = true;
      record_heads[i].position = position;
      record_heads[i].age = 0;
    }
  }
  if (!found) {
    // TODO: this is dangerous!
    record_heads.push_back(RecordHeadTrace(module_id, position));
  }
}

