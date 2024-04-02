#include "buffered.hpp"

std::shared_ptr<Buffer> findClosestMemory(Module* leftModule) {
  while (true) {
    if (!leftModule) return nullptr;
    if (leftModule->model == modelMemory) {
      return dynamic_cast<BufferedModule*>(leftModule)->getHandle()->buffer;
    }
    auto m = leftModule->model;
    if ((m == modelRuminate) ||
        (m == modelEmbellish) ||
        (m == modelDepict)) {  // This will be a list soon...
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
    if (abs(record_heads[i].position - position) <= NEAR_DISTANCE) {
      // WARN("head = %d, pos = %d", record_heads[i].position, position);
      return true;
    }
    if (abs(record_heads[i].position + length - position) <= NEAR_DISTANCE) {
      // WARN("head = %d, pos = %d", record_heads[i].position, position);
      return true;
    }
    if (abs(record_heads[i].position - (position + length)) <= NEAR_DISTANCE) {
      // WARN("head = %d, pos = %d", record_heads[i].position, position);
      return true;
    }
  }
  return false;
}

void Buffer::SetDirty(int position) {
  dirty[(int) floor(position / ((double) length / WAVEFORM_SIZE))] = true;
}

// Returns distance if near a recording head, except for the recording head
// with 'module_id', or INT_MAX if not considered "near".
// Typically called for the benefit of recording heads.
int Buffer::NearHeadButNotThisModule(int position, long long module_id) {
  // TODO: not sure if correct when near the ends of the buffer.
  int nearest = INT_MAX;
  for (int i = 0; i < (int) record_heads.size(); ++i) {
    if (record_heads[i].module_id == module_id) {
      continue;
    }
    nearest = std::min(nearest, abs(record_heads[i].position - position));
    nearest = std::min(nearest, abs(record_heads[i].position + length - position));
    nearest = std::min(nearest, abs(record_heads[i].position - (position + length)));
  }
  return nearest <= NEAR_DISTANCE ? nearest : INT_MAX;
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

  // Update position for this Set() call's module.
  // Memory creates and updates this list during the module scan in
  // process().
  // If we don't find it now, we will when it updates.
  for (int i = 0; i < (int) record_heads.size(); ++i) {
    if (record_heads[i].module_id == module_id) {
      record_heads[i].position = position;
      record_heads[i].age = 0;
      break;
    }
  }
}

