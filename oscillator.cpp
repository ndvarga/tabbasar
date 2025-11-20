#include "oscillator.h"

// Constructor taking arguments for sample rate and table data
Oscillator::Oscillator(Waveshape waveshape, unsigned int sampleRate, unsigned int wavetableSize, unsigned int nHarmonics, bool useInterpolation) {
	setup(waveshape, sampleRate, wavetableSize, useInterpolation);
} 

void Oscillator::setup(Waveshape waveshape, unsigned int sampleRate, unsigned int wavetableSize, unsigned int nHarmonics, bool useInterpolation)
{
	// It's faster to multiply than to divide on most platforms, so we save the inverse
	// of the sample rate for use in the phase calculation later
	inverseSampleRate_ = 1.0 / sampleRate;

	// resize the buffer to have the necessary number of wavetables
  wavetables_.resize(nHarmonics);

  //set it up
  for (unsigned int i = 0; i < wavetables_.size(); i++)
  {
    wavetables_[i].setup(sampleRate, wavetableSize, true);
  }

  // use an arbitrary fundamental frequency
  f_fundamental_ = 10;

  // Copy other parameters
	setWaveshape(waveshape);
	useInterpolation_ = useInterpolation;
	
	// Initialise the starting state
	readPointer_ = 0;
}

Oscillator::Waveshape Oscillator::getWaveshape()
{
	return table_type_;
}

void Oscillator::incrementWaveshape()
{
	switch(table_type_)
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

void Oscillator::setWaveshape(Waveshape waveshape)
{
	table_type_ = waveshape;
	
	
	// create the wavetable object
	// TODO: can make a vector of wavetables that we interpolate between

	
	//TODO SWITCH CASE
	switch (table_type_)
	{
		case (square):
		{
			// generate square wavetable
			for (unsigned int i = 0; i < wavetables_.size(); i++)
      {
        wavetables_[i].setAmplitude((float)1.0/i);
        wavetables_[i].setFrequency((float)f_fundamental_ * i);
      }
			
			break;
		}
		
		case (saw):
		{
			
			break;
		
		}
		
		case(sine):
		{
			
			break;
		}
		
		
		
		case(triangle):
		{
			//generate triangle wave
			// needs to rise and fall in half a period
		
			
			break;
		}
		
		case(noise):
			;
		
	}
	
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