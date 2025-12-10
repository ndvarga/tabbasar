#include "Piano.h"
#include <cmath>
#include <algorithm>


Piano::Piano() {
    setup();
}


void Piano::setup() {
  pianoSamples_.fill(kPianoMax_); // initialize the array to 0.0f
  pianoSum_ = kPianoMax_;         // initialize sum to 0.0f
  pianoIndex_ = 0;          // reset index
}

float Piano::process(float newValue) {
  // Remove the oldest sample from sum
  pianoSum_ -= pianoSamples_[pianoIndex_];
  
  // Add the new sample
  pianoSamples_[pianoIndex_] = newValue;
  pianoSum_ += newValue;
  
  // Move to next position (circular)
  pianoIndex_ = (pianoIndex_ + 1) % kPianoVectorSize;
  
  // Calculate average
  pianoAverage_ = pianoSum_ / kPianoVectorSize;
  return pianoAverage_;
}

unsigned int Piano::getSemitoneOffset(float mappedValue) {
  // Clamp the piano value to valid range
  if (mappedValue < kPianoMin_) mappedValue = kPianoMin_;
  if (mappedValue > kPianoMax_) mappedValue = kPianoMax_;
  
  // Calculate which semitone segment this value falls into
  // Each semitone covers kSemitoneStep of the piano range
  float roundedValue = std::roundf(mappedValue);
  unsigned int semitone = static_cast<unsigned int>(roundedValue);
  
  // Clamp to valid range (0-12)
  if (semitone >= kNumSemitones_) semitone = kNumSemitones_;
  
  return semitone;
}
