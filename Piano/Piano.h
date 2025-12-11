#pragma once
#include <array>
#include <numeric>
#include <Bela.h>
#include <cmath>

#define kPianoPin 7
#define kPianoVectorSize 8

class Piano {
  public:
    Piano();
    ~Piano() {}
    void setup();
    void process(float newValue);
    int getSemitoneOffset();

  private:
    // methods
    unsigned int findSemitoneOffset_(float mappedValue);

    // variables
    unsigned int pianoIndex_ = 0;
    // Use 12 as “no note” sentinel; avoids large wrapped value from -1 on unsigned
    unsigned int semitoneOffset_ = 12;


    // Lookup table: maps piano value ranges to semitone offsets (0-12)
    // Divides 0-12 range into 12 equal segments
    float kPianoMax_ = 12.0f;
    float kPianoMin_ = 0.0f;
    unsigned int kNumSemitones_ = 12; // 0-11 inclusive
};
