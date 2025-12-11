#include "Piano.h"
#include <cmath>
#include <algorithm>


Piano::Piano() {
    setup(12.0f);
}


void Piano::setup(float initValue) {
  pianoSamples_.fill(initValue); // initialize the array to 0.0f
  pianoSum_ = initValue;         // initialize sum to 12.0f
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
  pianoAverage_ = pianoSum_ / (float)kPianoVectorSize;
  return pianoAverage_;
}

unsigned int Piano::getSemitoneOffset(float mappedValue) {
  // Clamp the piano value to valid range
  const float kMin = 0.0f;
  const float kMax = 12.0f;
  if (mappedValue < kMin) mappedValue = kMin;
  if (mappedValue > kMax) mappedValue = kMax;
  
  // Calculate which semitone segment this value falls into
  // Each semitone covers kSemitoneStep of the piano range
  float roundedValue = roundf(mappedValue);
  unsigned int semitone = (unsigned int)(roundedValue);
  
  // Clamp to valid range (0-12)
  const unsigned int kNumSemitones = 12;
  if (semitone >= kNumSemitones) semitone = kNumSemitones;
  
  return semitone;
}
