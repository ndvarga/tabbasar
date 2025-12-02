#pragma once

#include "wavetable.h"
#include <vector>
#include <cmath>


class Oscillator 
{
  public:
    typedef enum
      {
        sine,
        square,
        saw,
        triangle,
        noise
      } Waveshape;
    

    Oscillator() {}
    Oscillator(Waveshape, unsigned int, unsigned int, unsigned int, bool); 
    
    void setup(Waveshape, unsigned int, unsigned int, unsigned int, bool); 		
    
    void setFundamentalFrequency(float f);	// Set the oscillator frequency
    float getFundamentalFrequency();		// Get the oscillator frequency
    
    void setWaveshape(Waveshape);
    Waveshape getWaveshape();
    void incrementWaveshape();
    
    
    float process();				// Get the next sample and update the phase
    
    ~Oscillator() {}				// Destructor

    float masterAmplitude;

private:
	std::vector<Wavetable> wavetables_;	// Buffer holding the wavetable
	Waveshape table_type_;              // the type of wavetable
  unsigned int n_harmonics_;          // number of harmonics the oscillator has
  unsigned int sampleRate_;           // Sample rate 
	float f_fundamental_;			          // Fundamental frequency of the oscillator
};
