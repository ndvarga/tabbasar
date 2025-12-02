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
	
	
	
	Wavetable() {}													// Default constructor
	Wavetable(float, unsigned int, bool); // constructor with setup arguments
	
	void setup(float, unsigned int, bool); 	 //setup function	
	
	void setFrequency(float);	// Set the oscillator frequency
	float getFrequency();		// Get the oscillator frequency
	
	void setAmplitude(float);

	float process();				// Get the next sample and update the phase
	
	~Wavetable() {}				// Destructor

private:
	void _drawTable();
	std::vector<float> table_; // Buffer holding the wavetable
	float inverseSampleRate_;	 // 1 divided by the audio sample rate	
	float frequency_;			     // Frequency of the wavetable
	float readPointer_;				 // Location of the read pointer (phase of oscillator)
	float amplitude_;       	 //amplitude of the wavetable
	bool useInterpolation_;		 // Whether to use linear interpolation
};