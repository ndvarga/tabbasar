#include "Piano.h"
#include <cmath>
#include <algorithm>


Piano::Piano() {
    setup();
}


void Piano::setup() {
  pianoIndex_ = 0;          // reset index
}

void Piano::process(float newValue) {
    semitoneOffset_ = findSemitoneOffset_(newValue);
}

int Piano::getSemitoneOffset() {
    if (semitoneOffset_ == 12) return -1;
	
	return semitoneOffset_;
}


unsigned int Piano::findSemitoneOffset_(float mappedValue) {
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
