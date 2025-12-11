#pragma once
#include <array>
#include <numeric>
#include <Bela.h>
#include <cmath>

#define kPianoPin 4
#define kPianoVectorSize 8

class Piano {
  public:
    Piano();
    ~Piano() {}
    void setup(float initValue);
    float process(float newValue);
    unsigned int getSemitoneOffset(float pianoAverage);

  private:
    unsigned int pianoIndex_ = 0;
    std::array<float, kPianoVectorSize> pianoSamples_;
    float pianoSum_;
    float pianoAverage_;
    
    // Lookup table: maps piano value ranges to semitone offsets (0-12)
    // Divides 0-12 range into 12 equal segments
    float kPianoMax_ = 12.0f;
    float kPianoMin_ = 0.0f;
    unsigned int kNumSemitones_ = 12; // 0-11 inclusive
};
