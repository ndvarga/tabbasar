/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

The platform for ultra-low latency audio and sensor processing
http://bela.io

(c) 2020 Augmented Instruments Ltd

A project from the Augmented Instruments Laboratory within the
Centre for Digital Music at Queen Mary University of London.
http://instrumentslab.org

The Bela software is distributed under the GNU Lesser General Public License
(LGPL 3.0), available here: https://www.gnu.org/licenses/lgpl-3.0.txt
*/

#include <Bela.h>
#include <libraries/Scope/Scope.h>
#include <cmath>
#include <vector>

#include "wavetable.h"	// This is needed for the Wavetable class

// Constants that define the program behaviour
const unsigned int kWavetableSize = 512;
const float kAmplitude = 0.1;
const float kDetune = 0.005;

const int kButtonPin = 1;				


// Browser-based oscilloscope
Scope gScope;

// Wavetable oscillators
Wavetable gOscillators[2];

// Step sequencer contents
std::vector<float> gSequencerBuffer = {36, 40, 43};
unsigned int gSequencerLocation = 0;

// Last state of the button
int gLastButtonStatus = HIGH;

bool setup(BelaContext *context, void *userData)
{
	std::vector<float> wavetable;
	
	// Check that audio and digital have the same number of frames
	// per block, an assumption made in render()
	if(context->audioFrames != context->digitalFrames) 
	{
		rt_fprintf(stderr, "This example needs audio and digital running at the same rate.\n");
		return false;
	}
		
	// Populate a buffer with the first 32 harmonics of a sawtooth wave
	wavetable.resize(kWavetableSize);
	for(unsigned int n = 0; n < wavetable.size(); n++) 
	{
		wavetable[n] = 0;
		for(unsigned int harmonic = 1; harmonic <= 32; harmonic++) 
		{
			wavetable[n] += sinf(2.0 * M_PI * (float)harmonic * (float)n / 
								 (float)wavetable.size()) / (float)harmonic;
		}
	}
	
	// Initialise the wavetable, passing the sample rate and the buffer
	for(unsigned int i = 0; i < 2; i++) {
		gOscillators[i].setup(context->audioSampleRate, wavetable);
	}

	// Set up the oscilloscope
	gScope.setup(2, context->audioSampleRate);
	
	// Set up the digital pins
	pinMode(context, 0, kButtonPin, INPUT);

	return true;
}

void render(BelaContext *context, void *userData)
{
    for(unsigned int n = 0; n < context->audioFrames; n++) {
    	float out = 0;
    	
    	// Get current frequency based on where we are in the sequencer
    	float midiNote = gSequencerBuffer[gSequencerLocation];
    	// non-linear rescaling, from midi note to frequency!
    	float frequency = 440.0 * powf(2.0, (midiNote - 69.0) / 12.0);
    	
    	// Calculate frequences of each of two oscillators
		float frequencies[2];
		frequencies[0] = frequency * (1.0 + kDetune);
		frequencies[1] = frequency * (1.0 - kDetune);

    	for(unsigned int i = 0; i < 2; i++) {
    		gOscillators[i].setFrequency(frequencies[i]);
			out += kAmplitude * gOscillators[i].process();
    	}           
		    
	
        // Check for a button press to advance the sequence
        int buttonStatus = digitalRead(context, n, kButtonPin);
        if(buttonStatus == LOW && gLastButtonStatus == HIGH)
        {
        	
  
        	gSequencerLocation++;
        	if(gSequencerLocation >= gSequencerBuffer.size())
        		gSequencerLocation = 0;
        }
		
        // Update the previous state of the button based on the current state
		gLastButtonStatus = buttonStatus;

		// Write the sample to every audio output channel            
    	for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) {
    		audioWrite(context, n, channel, out);
    	}
    			
    	// Write the output to the oscilloscope
    	gScope.log(out, buttonStatus);    
    }
}

void cleanup(BelaContext *context, void *userData)
{
	
}
