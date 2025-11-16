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

// wavetable.cpp: file for implementing the wavetable oscillator class

#include <cmath>
#include "wavetable.h"
#include <vector>

// Constructor taking arguments for sample rate and table data
Wavetable::Wavetable(Waveshape waveshape, float sampleRate, unsigned int wavetableSize, bool useInterpolation) {
	setup(waveshape, sampleRate, wavetableSize, useInterpolation);
} 

void Wavetable::setup(Waveshape waveshape, float sampleRate, unsigned int wavetableSize, bool useInterpolation)
{
	// It's faster to multiply than to divide on most platforms, so we save the inverse
	// of the sample rate for use in the phase calculation later
	inverseSampleRate_ = 1.0 / sampleRate;
	
	table_.resize(wavetableSize);
	// Copy other parameters
	setWaveshape(waveshape);
	useInterpolation_ = useInterpolation;
	
	// Initialise the starting state
	readPointer_ = 0;
}

Wavetable::Waveshape Wavetable::getWaveshape()
{
	return _table_type;
}

void Wavetable::incrementWaveshape()
{
	switch(_table_type)
	{
		case(sine):
		{
			setWaveshape(saw);
			break;
		}
		
		case(saw):
		{
			setWaveshape(square);
			break;
		}
		
		case(square):
		{
			setWaveshape(triangle);
			break;
		}
		
		case(triangle):
		{
			setWaveshape(sine);
			break;
		}
		case(noise):
		{
			setWaveshape(sine);
			break;
		}
	}
}

void Wavetable::setWaveshape(Waveshape waveshape)
{
	_table_type = waveshape;
	std::vector<float> wavetable;
	wavetable.resize(table_.size());
	
	// create the wavetable object
	// TODO: can make a vector of wavetables that we interpolate between

	int wavetableSize = table_.size();
	
	//TODO SWITCH CASE
	switch (_table_type)
	{
		case (square):
		{
			// generate square wavetable
			
			for (unsigned int i = 0; i < wavetableSize / 2; i++)
			{
				wavetable[i] = -1.0;
			}
			
			for (unsigned int i = wavetableSize / 2; i < wavetableSize; i++)
			{
				wavetable[i] = 1.0;
			}
			break;
		}
		
		case (saw):
		{
			//generate saw
			float increment = 2.0/wavetableSize;
			float adder = -1.0;
			
			for (unsigned int i = 0; i < wavetableSize; i++)
			{
				adder += increment;
				wavetable[i] = adder;
			}
			break;
		
		}
		
		case(sine):
		{
			for (unsigned int i = 0; i < wavetableSize; i++)
			{
				wavetable[i] = sinf(2.0 * M_PI * (float)i / (float)wavetable.size());
				
			}
			
			break;
		}
		
		
		
		case(triangle):
		{
			//generate triangle wave
			// needs to rise and fall in half a period
			float increment = 2.0/wavetableSize * 2.0;
			float adder = -1.0;
			
			// for (unsigned int i = 0; i < wavetableSize / 2; i++)
			// {
			// 	wavetable[i] = adder;
			// 	adder+=increment;
			// }
			
			// for (unsigned int i = wavetableSize / 2; i < wavetableSize; i++)
			// {
			// 	wavetable[i] = adder;
			// 	adder -= increment;
			// 	// wavetable[i] = 1.0 - 4.0* (float)(i - wavetableSize/2);
			// }
			
			for(unsigned int n = 0; n < wavetableSize/2; n++) 
			{
				wavetable[n] = -1.0 + 4.0 * (float)n / (float)wavetableSize;
			}
			for(unsigned int n = wavetableSize/2; n < wavetableSize; n++) 
			{
				wavetable[n] = 1.0 - 4.0 * (float)(n - wavetableSize/2) / (float)wavetableSize;
			}
			break;
		}
		
		case(noise):
			;
		
	}
	
	table_ = wavetable;
}

// Set the oscillator frequency
void Wavetable::setFrequency(float f) {
	frequency_ = f;
}

// Get the oscillator frequency
float Wavetable::getFrequency() {
	return frequency_;
}		

	
// Get the next sample and update the phase
float Wavetable::process() {
	
	// TODO: ADD ANTIALIASING
	
	float out = 0;
	
	// Make sure we have a valid table
	if(table_.size() == 0)
		return out;
	
	// Increment and wrap the phase
	readPointer_ += table_.size() * frequency_ * inverseSampleRate_;
	while(readPointer_ >= table_.size())
		readPointer_ -= table_.size();
	
	if(useInterpolation_) {
		// The pointer will take a fractional index. Look for the sample on
		// either side which are indices we can actually read into the buffer.
		// If we get to the end of the buffer, wrap around to 0.
		int indexBelow = floorf(readPointer_);
		int indexAbove = indexBelow + 1;
		if(indexAbove >= table_.size())
			indexAbove = 0;
	
		// For linear interpolation, we need to decide how much to weigh each
		// sample. The closer the fractional part of the index is to 0, the
		// more weight we give to the "below" sample. The closer the fractional
		// part is to 1, the more weight we give to the "above" sample.
		float fractionAbove = readPointer_ - indexBelow;
		float fractionBelow = 1.0 - fractionAbove;
	
		// Calculate the weighted average of the "below" and "above" samples
	    out = fractionBelow * table_[indexBelow] +
	    	  fractionAbove * table_[indexAbove];
	}
	else {
		// Read the table without interpolation
		out = table_[(int)readPointer_];
	}
	
	return out;
}			