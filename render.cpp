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
#include <libraries/Biquad/Biquad.h>
#include "Filter.h"
#include "oscillator.h"	// This is needed for the oscillator class
#include "debouncer.h"
#include "parameters.h"

// Constants that define the program behaviour
const unsigned int kWavetableSize = 512;

Biquad lowpass;

// Browser-based oscilloscope
Scope gScope;

// Wavetable oscillator
Oscillator gOscillators[2];
Oscillator gTestOsc;

//add vector for all of our Waveshapes to use in our oscillator
// std::vector<Wavetable::Waveshape> discreet_waveshapes = { Wavetable::Waveshape::sine, Wavetable::Waveshape::square, Wavetable::Waveshape::saw, Wavetable::Waveshape::triangle };

Debouncer gButtonDebouncer;

// Step sequencer contents
std::vector<float> gSequencerBuffer = {36, 40, 43};
unsigned int gSequencerLocation = 0;

// Oscillators parameters
float gAmplitude;
float gFrequencies[2];

// button parameters
// State machine states


int glastOscButtonState = LOW;
float gDebounceTimeMs = 50;
int gDebounceState = kStateOpen;  // initial state of debounce machine
int gDebounceCounter = 0;	// counter to exit lock state
int gDebounceInterval;	// duration of lock state



unsigned int gSampleTimer = 0;

bool setup(BelaContext *context, void *userData)
{
	std::vector<float> wavetable;
	
	// create a biquad lowpass filter
	Biquad::Settings lowpass_settings {.fs = context->audioSampleRate, .type = Biquad::lowpass, .q=0.707};
	lowpass.setup(lowpass_settings);

	if(context->analogSampleRate != context->audioSampleRate/2)
	{
		fprintf(stderr, "Error: for this project the sample rate of the analog inputs has to be half the audio sample rate (8 analog inputs)\n");
		return false;
	}
		
	
	
	// Initialise the wavetable, passing the sample rate and the buffer
	Oscillator::Waveshape oscillatorType = Oscillator::Waveshape::square;
	// arguments are oscillatorType, sample rate, wavetable size, 
	// for(unsigned int i = 0; i < 2; i++)
	
	//setup(waveshape, sampleRate, wavetableSize, nHarmonics, useInterpolation)
	gTestOsc.setup(oscillatorType, context->audioSampleRate, kWavetableSize, 32, false);
	// Set up the oscilloscope
	gScope.setup(1, context->audioSampleRate);
    
    // set the debounce interval in samples
    gDebounceInterval = context->audioSampleRate * gDebounceTimeMs / 1000.0;
	gTestOsc.setFundamentalFrequency(520.0);

	//button debouncer setup
	gButtonDebouncer.setup(gDebounceTimeMs, context->audioSampleRate);

	return true;
}

void render(BelaContext *context, void *userData)
{

    for(unsigned int n = 0; n < context->audioFrames; n++) 
    {

    	// read analog ins and update control parameters only every other frame
    	// because the analog sample rate is half of the audio one
    	if( !(n % 2) )
    	{
				float input0 = analogRead(context, n/2, 0);	// read analog in 0
				float input1 = analogRead(context, n/2, 1);	// read analog in 1
				float input2 = analogRead(context, n/2, 2);	// read analog in 2
				float input3 = analogRead(context, n/2, 3); // read analog in 3
				
				float frequency = map(input0, 0, 3.3 / 4.096, 55, 440);		// Frequency is first knob (analog in 0)
				float level = map(input1, 0, 3.3 / 4.096, -60, -20);		// Level is second knob (analog in 1)	
				// this third parameter is ready to be used
				float detune  = map(input2, 0, 3.3 / 4.096, 0, 0.05);	    // Detune is third knob (analog in 2)	
				float lowpass_frequency = map(input3, 0, 3.3/4.096, 1, 5000);
				
				lowpass.setFc(lowpass_frequency);
				
				gAmplitude = powf(10.0, level / 20);	// Convert level to linear amplitude
		
				// Compute frequencies from central freq and detune		
				gFrequencies[0] = frequency * (1.0 + detune);
				gFrequencies[1] = frequency * (1.0 - detune);
    	}
	
		unsigned int input0 = digitalRead(context,n,0);
		

		gTestOsc.incrementWaveshape();

		
		

		float oscillator_out = 0;
		float out = 0;
    	
   // 	for(unsigned int i = 0; i < 2; i++) 
   // 	{
   // 		gOscillators[i].setFundamentalFrequency(gFrequencies[i]);
			// oscillator_out += gAmplitude * gOscillators[i].process();
			
   // 	}
		
    	oscillator_out += gTestOsc.process();
    	out = oscillator_out;
    	// out = lowpass.process(oscillator_out);
    	
    	gSampleTimer++;
    	if (gSampleTimer > context->audioSampleRate / 10)
    	{
    		gSampleTimer = 0;
    		
    		// rt_printf("out = %f\n", out);
    	}
    	
    	for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) 
    	{
				// Write the sample to every audio output channel
    		audioWrite(context, n, channel, out);
    	}
    	
    	// Write the output to the oscilloscope
    	gScope.log(out);    	
    }
}

void cleanup(BelaContext *context, void *userData)
{
	
}
