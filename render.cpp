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
#include <algorithm>

// Step sequencer pins
const unsigned int kTempoInput = 0;			// Which analog input to read

//global variables for digital button pins

const int kButtonPin = 1;	
const int kReverseButtonPin = 2;

// Step sequencer contents

// Initialize with size
std::vector<std::vector<int>> matrix(4, std::vector<int>(4, 0));  // 3x4 matrix of zeros



std::vector<std::vector<float>> gSequencerPatterns = {
    {36, 40, 43},  // Pattern 0
    {36, 39, 43},  // Pattern 1
    // {36, 40, 43, 47},  // dominant
   
    //{880.0, 987.77, 1046.5, 1174.66}  // Pattern 2
};

unsigned int gSequencerLocation = 0;

int gLastButtonStatus = HIGH;
int gLastReverseButtonStatus = HIGH;


// global variables here for counting time
unsigned int gSampleCounter = 0;
unsigned int gMetroInterval;

int gCurrentPattern = 0;
float freq = gSequencerPatterns[gCurrentPattern][gSequencerLocation];




// Constants that define the program behaviour
const unsigned int kWavetableSize = 128;
const unsigned int kNumHarmonics = 32;

Biquad lowpass;

// Browser-based oscilloscope
Scope gScope;

// Wavetable oscillator
const unsigned int kDualOscillators = 1;

Oscillator gOscillators[kDualOscillators];
// Oscillator gTestOsc;

// A button deboucer for button 0
Debouncer gButtonDebouncer;



// Oscillators parameters
float gAmplitude;
float gFrequencies[kDualOscillators];



// button parameters



// State machine states


float gDebounceTimeMs = 50;
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
	
	
	//oscillator setup args are (waveshape, sampleRate, wavetableSize, nHarmonics, useInterpolation)
	gOscillators[0].setup(oscillatorType, context->audioSampleRate, kWavetableSize, kNumHarmonics, true);
	
	// temporary for no error
	gOscillators[0].setFundamentalFrequency(440);
	// gOscillators[1].setup(oscillatorType, context->audioSampleRate, kWavetableSize, kNumHarmonics, true);

	// Set up the oscilloscope
	gScope.setup(1, context->audioSampleRate);
    
    // set the debounce interval in samples
    gDebounceInterval = context->audioSampleRate * gDebounceTimeMs / 1000.0;
	// gTestOsc.setFundamentalFrequency(320.0);

	//button debouncer setup, with debounce time and audio sample rate
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
    		// code for arpegiiator
			float input = analogRead(context, n/2, kTempoInput); // read analog in 0
    		float bpm = map(input, 0, 3.3/4.096, 40, 1000); // turn into BPM
    		gMetroInterval = 80.0 * context->audioSampleRate / bpm;
    		
    		//TODO: MAKE WHAT THESE DIALS DO DEPENDENT ON THE BUTTON STATE
    		
			// float input0 = analogRead(context, n/2, 0);	// read analog in 0
			float input1 = analogRead(context, n/2, 1); // analog in 1 is level
			// float input2 = analogRead(context, n/2, 2);	// read analog in 2
			float input3 = analogRead(context, n/2, 3); // read analog in 3
			
			// float frequency = map(input0, 0, 3.3 / 4.096, 55, 880);		// Frequency is first knob (analog in 0)
			float level = map(input1, 0, 3.3 / 4.096, -60, -20);		// Level is second knob (analog in 1)	
			// // this third parameter is ready to be used
			// float detune  = map(input2, 0, 3.3 / 4.096, 0, 0.05);	    // Detune is third knob (analog in 2)	
			float lowpass_frequency = map(input3, 0, 3.3/4.096, 1, 5000); // use pin3 for lowpass_frequency
			
			lowpass.setFc(lowpass_frequency);
			
			gAmplitude = powf(10.0, level / 20);	// Convert level to linear amplitude
	
    	}
	
		unsigned int input0 = digitalRead(context,n,0);
    	
    	int reverseButtonStatus = digitalRead(context, n, kReverseButtonPin);
		if(reverseButtonStatus == LOW && gLastReverseButtonStatus == HIGH)
		//reverses all gSequencerPatterns when button in digital 2 is pressed
		for(int i = 0; i < gSequencerPatterns.size(); i++)
		{
    		std::reverse(gSequencerPatterns[i].begin(), gSequencerPatterns[i].end());
		}
		gLastReverseButtonStatus = reverseButtonStatus;
    	
    	
    	// Use button to change which GPattern is used
    	int buttonStatus = digitalRead(context, n, kButtonPin);
		if(buttonStatus == LOW && gLastButtonStatus == HIGH)
		{
			gCurrentPattern++;
    		if(gCurrentPattern >= gSequencerPatterns.size())
        	gCurrentPattern = 0;
		}
		gLastButtonStatus = buttonStatus;
    	
    	
    	
    	// Get current frequency based on where we are in the sequencer
    	float midiNote = gSequencerPatterns[gCurrentPattern][gSequencerLocation];
    	float frequency = 440.0 * powf(2.0, (midiNote - 69.0) / 12.0);
    	
    	gOscillators[0].setFundamentalFrequency(frequency);
    	// check if enough time has elapsed and increment the sequence location 
    	gSampleCounter++;
		
		if(gSampleCounter >= gMetroInterval)
		{
			gSampleCounter = 0;
			gSequencerLocation++;
			
			// wrap aroudn
			if(gSequencerLocation >= gSequencerPatterns[gCurrentPattern].size())
				gSequencerLocation = 0;
		}
		
		
		bool result = gButtonDebouncer.step(input0);
		if (result ==  true) {
			rt_printf("Button pressed\n");
			gOscillators[0].incrementWaveshape();
			// gOscillators[1].incrementWaveshape();
		}
		

		float oscillator_out = 0;
		float out = 0;
    	
		// for(unsigned int i = 0; i < kDualOscillators; i++) 
		// {
		// 	oscillator_out += gAmplitude * gOscillators[i].process();
				
		// }
		
    	// out = oscillator_out;
    	oscillator_out = gAmplitude * gOscillators[0].process();
    	out = lowpass.process(oscillator_out);
    	
    	gSampleTimer++;
    	if (gSampleTimer > context->audioSampleRate / 5)
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
