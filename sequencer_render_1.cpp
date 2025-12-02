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
#include <algorithm>

#include "Wavetable.h"	// This is needed for the Wavetable class

// Constants that define the program behaviour
const unsigned int kWavetableSize = 512;
const float kAmplitude = 0.1;
const float kDetune = 0.005;

const unsigned int kLedPin = 0;				// Digital out for LED
const unsigned int kInputTempo = 0;			// Which analog input to read

//global variables for switching patterns

const int kButtonPin = 1;	
const int kReverseButtonPin = 2;
int gLastButtonStatus = HIGH;
int gLastReverseButtonStatus = HIGH;


// global variables here for counting time
unsigned int gSampleCounter = 0;
unsigned int gMetroInterval;
unsigned int gLEDinterval;

// Browser-based oscilloscope
Scope gScope;

// Wavetable oscillators
Wavetable gOscillators[2];

// Step sequencer contents

//std::vector<std::vector<int>> matrix; //initializing matrix of patterns

// Initialize with size
std::vector<std::vector<int>> matrix(4, std::vector<int>(4, 0));  // 3x4 matrix of zeros

//std::vector<float> gSequencerBuffer = {36, 40, 43}; // for "On the run" use {40, 43, 47, 45, 43, 45, 50, 52};
//std::vector<float> gSequencerBufferm = {36, 39, 43};

std::vector<std::vector<float>> gSequencerPatterns = {
    {36, 40, 43},  // Pattern 0
    {36, 39, 43},  // Pattern 1
    // {36, 40, 43, 47},  // dominant
   
    //{880.0, 987.77, 1046.5, 1174.66}  // Pattern 2
};

unsigned int gSequencerLocation = 0;

int gCurrentPattern = 0;
float freq = gSequencerPatterns[gCurrentPattern][gSequencerLocation];



bool setup(BelaContext *context, void *userData)
{
	std::vector<float> wavetable;
	
	// Check that we have the expected number of analog inputs and outputs
	// because render() assumes half as many analog frames as audio frames
	if(context->audioFrames != 2*context->analogFrames) 
	{
		rt_fprintf(stderr, "This project needs analog I/O running at half the audio rate.\n");
		return false;
	}
	
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
	for(unsigned int i = 0; i < 2; i++) 
	{
		gOscillators[i].setup(context->audioSampleRate, wavetable);
	}

	// Set up the oscilloscope
	gScope.setup(1, context->audioSampleRate);
	
    // Set up the digital pins
	pinMode(context, 0, kLedPin, OUTPUT);
	
	// Initialise LED interval according to sample rate	
	gLEDinterval = 0.05 * context->audioSampleRate;

	return true;
}

void render(BelaContext *context, void *userData)
{
    for(unsigned int n = 0; n < context->audioFrames; n++) 
    {
    	
    	// Read Analog input to get the current tempo
    	if(n%2==0)
    	{
    		float input = analogRead(context, n/2, kInputTempo); // read analog in 0
    		float bpm = map(input, 0, 3.3/4.096, 40, 1000); // turn into BPM
    		gMetroInterval = 80.0 * context->audioSampleRate / bpm;
    	}
    	
    	float out = 0;
    	
    	
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
    	
    	// Calculate frequences of each of two oscillators
		float frequencies[2];
		frequencies[0] = frequency * (1.0 + kDetune);
		frequencies[1] = frequency * (1.0 - kDetune);

    	for(unsigned int i = 0; i < 2; i++) {
    		gOscillators[i].setFrequency(frequencies[i]);
			out += kAmplitude * gOscillators[i].process();
    	}
    	
    	// check if enough time has elapsed and increment the sequence location 
    	gSampleCounter++;
		
		if(gSampleCounter >= gMetroInterval)
		{
			gSampleCounter = 0;
			gSequencerLocation++;
			if(gSequencerLocation >= gSequencerPatterns[gCurrentPattern].size())
				gSequencerLocation = 0;
		}
		
		// Write the sample to every audio output channel            
    	for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) 
    	{
    		audioWrite(context, n, channel, out);
    	}
    	
    	// Turn on the LED on if we are early enough in the tick
		if(gSampleCounter < gLEDinterval)
			digitalWriteOnce(context, n, kLedPin, HIGH);
		else
			digitalWriteOnce(context, n, kLedPin, LOW);
    	
    	// Write the output to the oscilloscope
    	gScope.log(out);    
    }
}

void cleanup(BelaContext *context, void *userData)
{
	
}
