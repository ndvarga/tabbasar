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
#include <libraries/Gui/Gui.h>
#include <libraries/GuiController/GuiController.h>
#include <libraries/Scope/Scope.h>
#include <cmath>

#include <vector>
#include <libraries/Biquad/Biquad.h>
#include "oscillator.h"	// This is needed for the oscillator class
#include "Piano/Piano.h"
#include "AnalogDebouncer.h"
#include "parameters.h"
#include "ADSR.h"
#include "Debouncer.h"
#include "Ramp.h"
#include <algorithm>
#include <array>

//const int kPianoPin = 6;
// piano button reading
Piano gPiano;
AnalogDebouncer gPianoDebouncer;
int gMidiBase = 12; // MIDI base note
int gPianoSemitoneOffset = 0;

// ANALOG pin defs 
const int kDial0Pin = 0;			// Dial 0 (changes control depending on control mode)
const int kDial1Pin = 1;			// etc
const int kDial2Pin = 2;
const int kDial3Pin = 3;
const int kMasterVolumePin = 4;
const int kGlobalLowpassPin = 5;

//global variables for DIGITAL button pins
const int kLeftButtonPin = 0;
const int kRightButtonPin = 1;
const int kArpOnOffButtonPin = 2; // these always control the same thing no matter the mode
const int kControlModeTogglePin = 3;
const int kOctaveChangePin = 6; // octave change button


// global control mode
int gControlMode = ControlParameters::OSC_1_PARAMS;

// global dial values
float gDial0 = 0.1;
float gDial1 = 0.1;
float gDial2 = 0.1;
float gDial3 = 0.1;

// Step sequencer contents

// Initialize with size
// std::vector<std::vector<int>> matrix(4, std::vector<int>(4, 0));  // 3x4 matrix of zeros

std::vector<std::vector<float>> gSequencerPatterns = {
    {0, 4, 7, 4},  // Pattern 0
    {0, 3, 7, 3},  // Pattern 1
    // {36, 40, 43, 47},  // dominant
};

unsigned int gSequencerLocation = 0;

int gLastButtonStatus = HIGH;
int gLastReverseButtonStatus = HIGH;

bool gArpModeEnabled = false;  

unsigned int gSampleCounter = 0;
unsigned int gMetroInterval = 22050;

int gCurrentPattern = 0;
float freq = gSequencerPatterns[gCurrentPattern][gSequencerLocation];

const unsigned int kWavetableSize = 1024;
const unsigned int kNumHarmonics = 32;

Biquad lowpass;
Scope gScope;

const unsigned int kDualOscillators = 1; // Wavetable oscillator

Oscillator gOscillators[kDualOscillators]; // Oscillator gTestOsc;

//button debouncers for all our buttons :3
Debouncer gD0Debouncer; // Button debouncer object
Debouncer gD1Debouncer;

Debouncer gArpOnOffDebouncer;
Debouncer gControlModeDebouncer;

Debouncer gOctaveChangeDebouncer;

ADSR gAmplitudeADSR, gFilterADSR; // ADSR objects

Gui gGui;
GuiController gGuiController;

// Oscillators parameters
float gAmplitude;
float gFrequencies[kDualOscillators];


// State machine states


float gDebounceTimeMs = 50;
int gDebounceCounter = 0;	// counter to exit lock state
int gDebounceInterval;	// duration of lock state


unsigned int gSampleTimer = 0;

bool setup(BelaContext *context, void *userData)
{
	// init piano, 50 ms debounce time
	gPiano.setup();
	gPianoDebouncer.setup(context->analogSampleRate, .1, 0.7, 12.0f);

  // Initialise the ADSR objects
	gAmplitudeADSR.setSampleRate(context->audioSampleRate);

	// setup all digital pins as inputs
	// pinMode(context, 0, kLeftButtonPin, INPUT);
	// pinMode(context, 0, kRightButtonPin, INPUT);
	// pinMode(context, 0, kControlModeTogglePin, INPUT);
	// pinMode(context, 0,kArpOnOffButtonPin, INPUT);
	
	// Initialise the button debouncers with 50ms interval
	gD0Debouncer.setup(context->audioSampleRate, .05);
	gD1Debouncer.setup(context->audioSampleRate, .05);
	gArpOnOffDebouncer.setup(context->audioSampleRate, .05);
	gControlModeDebouncer.setup(context->audioSampleRate, .05);
	gOctaveChangeDebouncer.setup(context->audioSampleRate, .05);
	
	// Set up the GUI
	gGui.setup(context->projectName);
	gGuiController.setup(&gGui, "ADSR Controller");	
	
	// Arguments: name, minimum, maximum, increment, default value
	gGuiController.addSlider("Frequency", 220, 55, 440, 0);
	gGuiController.addSlider("Amplitude Attack time", 0.01, 0.001, 0.1, 0);
	gGuiController.addSlider("Amplitude Decay time", 0.05, 0.01, 0.3, 0);
	gGuiController.addSlider("Amplitude Sustain level", 0.3, 0, 1, 0);
	gGuiController.addSlider("Amplitude Release time", 0.2, 0.001, 2, 0);
	gGuiController.addSlider("Filter base frequency", 200, 50, 1000, 0);
	gGuiController.addSlider("Filter sensitivity", 3000, 0, 10000, 0);
	gGuiController.addSlider("Filter Q", 4, 0.5, 10, 0);
	gGuiController.addSlider("Filter attack time", 0.05, 0.001, 0.1, 0);
	gGuiController.addSlider("Filter decay time", 0.1, 0.01, 0.3, 0);
	gGuiController.addSlider("Filter sustain level", 0.6, 0, 1, 0);
	gGuiController.addSlider("Filter Release time", 0.3, 0.0001, 2, 0);
	
	
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
	// gTestOsc.setFundamentalFrequency(320.0);

	return true;
}

void render(BelaContext *context, void *userData)
{
		// Retrieve values from the sliders

		// float ampAttackTime = gGuiController.getSliderValue(1);
		// float ampDecayTime = gGuiController.getSliderValue(2);
		// float ampSustainLevel = gGuiController.getSliderValue(3);
		// float ampReleaseTime = gGuiController.getSliderValue(4);
		// float filterBase = gGuiController.getSliderValue(5);
		// float filterSensitivity = gGuiController.getSliderValue(6);
		// float filterQ = gGuiController.getSliderValue(7);
		// float filterAttackTime = gGuiController.getSliderValue(8);
		// float filterDecayTime = gGuiController.getSliderValue(9);
		// float filterSustainLevel = gGuiController.getSliderValue(10);
		// float filterReleaseTime = gGuiController.getSliderValue(11);

		// // Set oscillator and ADSR parameters
		// gAmplitudeADSR.setAttackTime(ampAttackTime);
		// gAmplitudeADSR.setDecayTime(ampDecayTime);
		// gAmplitudeADSR.setSustainLevel(ampSustainLevel);
		// gAmplitudeADSR.setReleaseTime(ampReleaseTime);
		// gFilterADSR.setAttackTime(filterAttackTime);
		// gFilterADSR.setDecayTime(filterDecayTime);
		// gFilterADSR.setSustainLevel(filterSustainLevel);
		// gFilterADSR.setReleaseTime(filterReleaseTime);
		// lowpass.setQ(filterQ);


    for(unsigned int n = 0; n < context->audioFrames; n++) 
    {
        // mode change code here
		// read control mode button to see if we have to change modes
		int rawControlMode = digitalRead(context, n, kControlModeTogglePin);
		gControlModeDebouncer.process(rawControlMode);

        int lastControlMode = gControlMode;

		if (gControlModeDebouncer.fallingEdge())
		{
			if (gControlMode < ControlParameters::ARP)
			{
				gControlMode++;		
			}
			else
			{
				gControlMode = 0;
			}
			// rt_printf("control mode = %d\n", gControlMode);
			switch(gControlMode) {
		    case OSC_1_PARAMS:
		        rt_printf("Switching to mode: OSC 1 PARAMS\n");
		        break;
		    case OSC_1_ADSR:
		        rt_printf("Switching to mode: OSC 1 ADSR\n");
		        break;
		    case ARP:
		        rt_printf("Switching to mode: ARP\n");
		        break;
		    default:
		        rt_printf("Switching to mode: UNKNOWN\n");
		        break;
			}
			
		}


    	// read analog ins and update control parameters only every other frame
    	// because the analog sample rate is half of the audio one
    	if( !(n % 2) )
    	{
    		//piano processing code here
    		float pianoRead = analogRead(context, n/2, kPianoPin);
    		float pianoValue = map(pianoRead, 0, 3.3/4.096, 0, 12);

    		float pianoDebounced = gPianoDebouncer.process(pianoValue);
    		if (pianoDebounced >= 0.0f && pianoDebounced < 11.5f) 
    		{
    			rt_printf("Piano value: %f\n", pianoDebounced);
				gPiano.process(pianoDebounced);
				int pianoSemitoneOffset = gPiano.getSemitoneOffset();

				if (pianoSemitoneOffset != -1) 
				{
					gPianoSemitoneOffset = pianoSemitoneOffset;
					rt_printf("Piano semitone offset: %d\n", pianoSemitoneOffset);
				}
    		}

    		// code for arpegiiator, using analog 0
    		// it should only change the value of something in a mode ONCE YOU CHANGE THE DIAL IN THAT MODE SPEICIFICALLY
			gDial0 = analogRead(context, n/2, kDial0Pin); // read analog in 0
			gDial1 = analogRead(context, n/2, kDial1Pin); // read analog in 1
			gDial2 = analogRead(context, n/2, kDial2Pin); // read analog in 2
			gDial3 = analogRead(context, n/2, kDial3Pin); // read analog in 3



 
			// global dials are done
			float globalLowpassRaw = analogRead(context, n/2, kGlobalLowpassPin);
			float masterLevel = analogRead(context, n/2, kMasterVolumePin);
			float level = map(masterLevel, 0, 3.3 / 4.096, -60, -10);		// Level is second knob (analog in 1)	
			// // this third parameter is ready to be used
			// float detune  = map(input2, 0, 3.3 / 4.096, 0, 0.05);	    // Detune is third knob (analog in 2)	
			float lowpass_frequency = map(globalLowpassRaw, 0, 3.3/4.096, 1, 8000); // use pin3 for lowpass_frequency
			
			lowpass.setFc(lowpass_frequency);
			
			gAmplitude = powf(10.0, level / 20);	// Convert level to linear amplitude
	    
    	}
		
		
		int leftButtonValue = digitalRead(context, n, kLeftButtonPin);
		int rightButtonValue = digitalRead(context, n, kRightButtonPin);
		int arpModeButtonStatus = digitalRead(context, n, kArpOnOffButtonPin);
		int octaveChangeButtonValue = digitalRead(context, n, kOctaveChangePin);

		// process global arp mode
		gArpOnOffDebouncer.process(arpModeButtonStatus);

		if(gArpOnOffDebouncer.fallingEdge()) 
		{
			if (gArpModeEnabled)
			{
				gFilterADSR.release();
				gAmplitudeADSR.release();				
			}

			gArpModeEnabled = !gArpModeEnabled;

			rt_printf("switched arpMode to %d\n", gArpModeEnabled);
		}    	

		gOctaveChangeDebouncer.process(octaveChangeButtonValue);
		if (gOctaveChangeDebouncer.fallingEdge())
		{
			if (gMidiBase > 36)
			{
				gMidiBase = 0;
			}
			else
			{
				gMidiBase += 12;
			}
			rt_printf("MIDI base note changed to %d semitones from C3\n", gMidiBase);
		}

		
		gD0Debouncer.process(leftButtonValue);
		gD1Debouncer.process(rightButtonValue);
			
		switch(gControlMode) 
		{
			case ControlParameters::OSC_1_PARAMS:
			{
				// temp center frequency control
				
				// if arpmode is off
				// starting note is C3

				if (!gArpModeEnabled)
				{
					float osc1_detune_semitones = map(gDial1, 0, 3.3/4.096, 0, 1);
					float osc1_detune_ratio = powf(2.0f, (osc1_detune_semitones/12.0f));
					
					float currentFrequency = gOscillators[0].getFundamentalFrequency();
					gOscillators[0].setFundamentalFrequency(currentFrequency*osc1_detune_ratio);
				}
				
				
				// digital stuff
				if (gD0Debouncer.fallingEdge())
				{
					//add octave change code here
					rt_printf("Dial 0 falling edge\n");
				}
				
				if (gD1Debouncer.fallingEdge())
				{
					// change if detune
					gOscillators[0].incrementWaveshape();
					rt_printf("Waveshape changed\n");

				}
				
				break;	
			}
			
			case ControlParameters::OSC_1_ADSR:
			{

				// can change ranges later
				float osc_1_attack = map(gDial0, 0, 3.3/4.096, 0.001, 0.5);
				float osc_1_decay = map(gDial1, 0, 3.3/4.096, 0.01, 0.3);
				float osc_1_sustain = map(gDial2, 0, 3.3/4.096, 0, 1);
				float osc_1_release = map(gDial3, 0, 3.3/4.096, 0.001, 0.5);
				
				gAmplitudeADSR.setAttackTime(osc_1_attack);
				gAmplitudeADSR.setDecayTime(osc_1_decay);
				gAmplitudeADSR.setSustainLevel(osc_1_sustain);
				gAmplitudeADSR.setReleaseTime(osc_1_release);
				
				
				break;
			}
			
			// // case ControlParameters::OSC_2_PARAMS:
			// {
				
			// 	break;
			// }
			
			// // case ControlParameters::ADDITIVE_NOISE:
			// {
			// 	break;
			// }
			
			case ControlParameters::ARP:
			{

				float bpm = map(gDial0, 0, 3.3/4.096, 40, 1000); // turn into BPM
	    		gMetroInterval = 80.0 * context->audioSampleRate / bpm;
				
				// direction change code
				if (gD0Debouncer.fallingEdge())
				{
					for(int i = 0; i < gSequencerPatterns.size(); i++)
					{
							std::reverse(gSequencerPatterns[i].begin(), gSequencerPatterns[i].end());
					}

				}
				// Use button to change which GPattern is used

				if(gD1Debouncer.fallingEdge())
				{
					gCurrentPattern++;
						if(gCurrentPattern >= gSequencerPatterns.size())
							gCurrentPattern = 0;
				}

				break;
			}
		}
			
			

			
		// Get current frequency based on where we are in the sequencer or if a piano is being played
		if (gArpModeEnabled)
		{
			float sequencerOffset = gSequencerPatterns[gCurrentPattern][gSequencerLocation];

			float frequency = 130.81 * powf(2.0, (sequencerOffset + gPianoSemitoneOffset + gMidiBase) / 12.0);
	    	gOscillators[0].setFundamentalFrequency(frequency);	    	

		}
		else 
		{
			int pianoSemitoneOffset = gPiano.getSemitoneOffset();
			if (gSampleCounter == 1 || gSampleCounter == 0)
			{
				rt_printf("semitone offset FAR AWAY = %d\n", pianoSemitoneOffset);
			}
			// if we have a new, non null note on the piano, trigger the ADSR and filter ADSR
			if ((pianoSemitoneOffset != -1)) {
				gPianoSemitoneOffset = pianoSemitoneOffset;
				float frequency = 130.81 * powf(2.0, (pianoSemitoneOffset + gMidiBase) / 12.0);
				rt_printf("frequency = %f\n", frequency);
				gOscillators[0].setFundamentalFrequency(frequency);
				
				gAmplitudeADSR.trigger();
				gFilterADSR.trigger();
				rt_printf("ADSRs triggered\n");
			}
		}
		
		// get the next value from the ADSR envelope
		float amplitude = gAmplitudeADSR.process();
			
		// set the filter frequency based on its ADSR
		// float filterControl = gFilterADSR.process();
		// lowpass.setFc(filterBase + filterSensitivity * filterControl);
		
    	// check if enough time has elapsed and increment the sequence location 
    	gSampleCounter++;
		

		if(gArpModeEnabled && gSampleCounter >= gMetroInterval)
			{
				gSampleCounter = 0;
				gSequencerLocation++;
			
				
				// can have an if statement to put the triggering here or at the beginning iof the sequence
				gAmplitudeADSR.trigger();
				gFilterADSR.trigger();
				// wrap aroudn
				if(gSequencerLocation >= gSequencerPatterns[gCurrentPattern].size())
				{
					gSequencerLocation = 0;
				}
		}

	
    	float oscillator_out = gAmplitude * amplitude * gOscillators[0].process();

		// use lowpass filter
		float out = lowpass.process(oscillator_out);
    	
    	
    	// nik's dumbass code
    	gSampleTimer++;
    	if (gSampleTimer > context->audioSampleRate)
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
