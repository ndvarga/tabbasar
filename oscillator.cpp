#include "oscillator.h"

// Constructor taking arguments for sample rate and table data
Oscillator::Oscillator(Waveshape waveshape, float sampleRate, unsigned int wavetableSize, unsigned int nHarmonics, bool useInterpolation) {
	setup(waveshape, sampleRate, wavetableSize, nHarmonics, useInterpolation);
} 

void Oscillator::setup(Waveshape waveshape, float sampleRate, unsigned int wavetableSize, unsigned int nHarmonics, bool useInterpolation)
{
	sampleRate_ = sampleRate;
	// resize the buffer to have the necessary number of wavetables
	wavetables_.resize(nHarmonics);
	n_harmonics_ = nHarmonics;
	
	
	//set it up
	for (unsigned int i = 0; i < wavetables_.size(); i++)
	{
		// Wavetable harmonic;
		// harmonic.setup(sampleRate, wavetableSize, useInterpolation);
		wavetables_[i].setup(sampleRate, wavetableSize, useInterpolation);
		
	}
	
	// use an arbitrary fundamental frequency
	f_fundamental_ = 10;
	
	// Copy other parameters
	setWaveshape(waveshape);
	
	// Initialise the starting state
}

Oscillator::Waveshape Oscillator::getWaveshape()
{
	return table_type_;
}

void Oscillator::incrementWaveshape()
{
	
	//waveshape state machine
	
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
			setWaveshape(noise);
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

	//TODO SWITCH CASE
	switch (table_type_)
	{
		case (square):
		{
			// generate square wavetable

			for (unsigned int i = 0; i < wavetables_.size(); i++)
	    	{
	    		wavetables_[i].setFrequency((float)f_fundamental_ * (i+1));

		        // If i is even, mute the harmonic
		        if ((i%2))
		        {
		          wavetables_[i].setAmplitude(0);
		        }
		        else
		        // Otherwise, set the frequency and amplitude
		        {
		    		wavetables_[i].setAmplitude(1.0f/(float)(i+1));

	        	}
    	}
			
			break;
		}
		
		case (saw):
		{
	      // x(saw) = -2/pi sum((-1)^k*sin(2*pi*f*t/k))
			for (unsigned int i = 0; i < wavetables_.size(); i++)
	    	{
	    		// multiply by (-1)^k
	        	float amplitude = -2 / M_PI * 1.0/(float)(i+1) * (i % 2 ? -1.0f : 1.0f);
	        	wavetables_[i].setAmplitude(amplitude);
	    		wavetables_[i].setFrequency(f_fundamental_ * (i+1));
	    	}
	
			break;
		
		}
		
		case(sine):
		{
			wavetables_[0].setAmplitude(1);
    		wavetables_[0].setFrequency(f_fundamental_);
	
	    	for (unsigned int i = 1; i < wavetables_.size(); i++)
	    	{
	        	wavetables_[i].setAmplitude(0);
	        	wavetables_[i].setFrequency(1.0f);
	    	}
			break;
		}
		
		
		case(triangle):
		{
			//generate triangle wave
			// needs to rise and fall in half a period
			for (unsigned int i = 0; i < wavetables_.size(); i++)
			{
	        // get odd harmonics
		        if (i % 2)
		        {
		          float amplitude = 8 / (M_PI * M_PI) * powf(-1.0f, (i - 1) / 2.0f) / powf((float)i, 2.0f);
		          wavetables_[i].setAmplitude(amplitude);
		          wavetables_[i].setFrequency(i * f_fundamental_);
		        }
		        else
		        {
		          // mute even harmonics
		          wavetables_[i].setAmplitude(0);
		          //set arbitrary frequency so there's no bugs
		          wavetables_[i].setFrequency(1.0f);
		        }

    		}
			break;
		}
		
		case(noise):
			;
		
	}
	rt_printf("set table_type_ to %d\n", table_type_);
	
}

// Set the oscillator frequency
void Oscillator::setFundamentalFrequency(float f) {
	f_fundamental_ = f;

	//change the fundamental frequency, then set the frequencies to the harmonic. 
	for (unsigned int i = 0; i < wavetables_.size(); i++)
	{
		float f_harmonic = f_fundamental_ * (i+1);
		wavetables_[i].setFrequency(f_harmonic);
		// anti-aliasing
		if (f_harmonic > (sampleRate_ / 2.0f))
		{
			wavetables_[i].setAmplitude(0);
			// rt_printf("muted harmonic %d", i+1);
		}
	}
}

// Get the oscillator frequency
float Oscillator::getFundamentalFrequency() {
	return f_fundamental_;
}		

	
// Get the next sample from all wavetables
float Oscillator::process() {
	
	
	float out = 0;
	
	if (table_type_ == Waveshape::noise)
	{
		// rand() returns 0..RAND_MAX; scale to [-1.0, 1.0]
		out = (float)std::rand() / (float)RAND_MAX * 2.0f - 1.0f;
    	return out;
	}
	
	else
	{
		// go through all wavetables
		for (unsigned int i = 0; i < wavetables_.size(); i++)
		{
			// anti aliasing
			if (wavetables_[i].getFrequency() < (sampleRate_ / 2.0f))
			{
				  float table_sample = wavetables_[i].process();
				  out += table_sample;
			}
		}
		
	}

	return out;
}			