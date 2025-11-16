/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

http://bela.io

C++ Real-Time Audio Programming with Bela - Lecture 5: Classes and Objects
wavetable-class: an example that implements a wavetable oscillator as a C++ class
*/

// Wavetable.h: header file for wavetable oscillator class

#pragma once

#include <vector>
#include <string>

class Wavetable {
public:
	typedef enum
	{
		sine,
		square,
		saw,
		triangle,
		noise
	} Waveshape;
	
	
	Wavetable() {}													// Default constructor
	Wavetable(Waveshape, float, unsigned int, bool);
	
	void setup(Waveshape, float, unsigned int, bool); 		
	
	void setFrequency(float f);	// Set the oscillator frequency
	float getFrequency();		// Get the oscillator frequency
	
	void setWaveshape(Waveshape);
	Waveshape getWaveshape();
	void incrementWaveshape();
	
	
	float process();				// Get the next sample and update the phase
	
	~Wavetable() {}				// Destructor

private:
	std::vector<float> table_;	// Buffer holding the wavetable
	Waveshape _table_type;
	float inverseSampleRate_;	// 1 divided by the audio sample rate	
	float frequency_;			// Frequency of the oscillator
	float readPointer_;			// Location of the read pointer (phase of oscillator)
	bool useInterpolation_;		// Whether to use linear interpolation
};