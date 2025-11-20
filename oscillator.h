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
    
    void setFrequency(float f);	// Set the oscillator frequency
    float getFrequency();		// Get the oscillator frequency
    
    void setWaveshape(Waveshape);
    Waveshape getWaveshape();
    void incrementWaveshape();
    
    
    float process();				// Get the next sample and update the phase
    
    ~Oscillator() {}				// Destructor

private:
	std::vector<Wavetable> wavetables_;	// Buffer holding the wavetable
	Waveshape table_type_;
  unsigned int n_harmonics_;
	float inverseSampleRate_;	// 1 divided by the audio sample rate	
	float f_fundamental_;			// Fundamental frequency of the oscillator
	float readPointer_;			// Location of the read pointer (phase of oscillator)
	bool useInterpolation_;		// Whether to use linear interpolation
};
