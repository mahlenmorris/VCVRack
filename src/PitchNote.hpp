#pragma once
#include <rack.hpp>

/* Lifted from Chinenual (https://github.com/chinenual/Chinenual-VCV) */

namespace Chinenual {

// min/max values assuming +/- 10v signals
static const float PITCH_VOCT_MIN = -10.f;
static const float PITCH_VOCT_MAX = 10.f;
static const int PITCH_NOTE_MIN = -60;  // MIDI note values
static const int PITCH_NOTE_MAX = 180;

enum NoteAccidental { SHARP, FLAT };
static std::vector<std::string> NoteAccidentalNames = {
    "Sharp",
    "Flat",
};

inline float pitchToVoltage(int note) {
  // based on VCV CORE MIDI_CV:
  return (note - 60.f) / 12.f;
}
inline float microPitchToVoltage(float note) {
  // based on VCV CORE MIDI_CV:
  return (note - 60.f) / 12.f;
}
inline int voltageToPitch(float v) {
  // based on VCV CORE CV_MIDI:
  return (int)std::round(v * 12.f + 60.f);
}
inline float voltageToMicroPitch(float v) {
  // based on VCV CORE CV_MIDI:
  return (v * 12.f + 60.f);
}
/* deviation from equal temperament (cents/100) */
inline float voltageToPitchDeviation(float v) {
  return voltageToMicroPitch(v) - (float)voltageToPitch(v);
}

inline float pitchDevToFrequencyDev(float pitchDeviation) {
  // an octave is 12 semitones per volt.  pitchDeviation is vs. a semitone
  // ("cents / 100") so 0.5 pitch deviaton is 1/24 of an octave, or 1/25V)
  return pitchDeviation / 12.f;
}

/* Careful: noteDeviation is scaled by note value - not by voltage */
inline void pitchToText(std::string& text, int note, float noteDeviation,
                        NoteAccidental accidentalMode) {
  // warning: note is not just in the MIDI range - might be much lower (-10v in
  // v/oct == -60 "note")

  // in case noteDeviation is larger than a semitone, apply it first and compute
  // a new "smaller than semitone" deviation:
  int n = std::round(note + noteDeviation);
  float nDeviation = noteDeviation - (n - note);

  std::vector<std::string> noteNames;
  if (accidentalMode == SHARP) {
    noteNames = {"C",  "C#", "D",  "D#", "E",  "F",
                 "F#", "G",  "G#", "A",  "A#", "B"};
  } else {
    noteNames = {"C",  "Db", "D",  "Eb", "E",  "F",
                 "Gb", "G",  "Ab", "A",  "Bb", "B"};
  }

  // ensure nameIndex is positive -- add several octaves to ensure we the
  // numerator is > 0.  n could be as low as -60.   But just in case we feed
  // this something way out of range, add even more.  (Bogaudio LLFO seems to
  // somehow be abnle to send +/- 12V?)
  int nameIndex = (1200 + n) % 12;

  int octave = (n / 12) - 1;  // simple division produces 60=C5; we want to
                              // display that as C4, so subtract 1
  auto absDeviation = std::abs(nDeviation);
  if (absDeviation >= 0.01f) {
    text = rack::string::f("%s%d %s%dc", noteNames[nameIndex].c_str(), octave,
                           (nDeviation > 0 ? "+" : "-"),
                           (int)(absDeviation * 100));
  } else {
    text = rack::string::f("%s%d", noteNames[nameIndex].c_str(), octave);
  }
}
}  // namespace Chinenual
