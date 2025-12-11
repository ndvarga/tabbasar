/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

http://bela.io

Analog Debouncer: debounces analog input values
Based on Debouncer class for digital inputs
*/

// AnalogDebouncer.h: simple class to debounce an analog input

#pragma once

class AnalogDebouncer {
private:
	// State machine states
	enum {
		kStateUnpressed = 0,
		kStateJustPressed,
		kStatePressed,
		kStateJustUnpressed
	};

public:
	// Constructor
	AnalogDebouncer() {};
	
	// Constructor specifying a sample rate, thresholds, debounce interval, and change threshold
	AnalogDebouncer(float sampleRate, float interval, float changeThreshold);
	
	// Set the sample rate, thresholds, debounce interval, and change threshold
	// interval: time in seconds for debouncing and lock duration
	// changeThreshold: minimum change in value required to engage lock (0 = disabled, uses absolute thresholds)
	void setup(float sampleRate, float interval, float changeThreshold);
	
	// Return the debounced value given the raw analog input
	float process(float rawInput);
	
	// Return the current debounced value
	float currentValue();
	
  bool justPressed();
  
  bool justUnpressed();
	
	// Return whether the lock is currently engaged
	bool isLocked();
	
	// Return the last input value
	float getLastInputValue();
	
	// Destructor
	~AnalogDebouncer();

private:
	// State variables, not accessible to the outside world
	int   currentState_;
	int   previousState_;
	int   counter_;
	int   debounceInterval_;
	float debouncedValue_; // Current debounced output value
	
	// Lock mechanism variables
	bool  isLocked_;       // Whether lock is currently engaged
	int   lockCounter_;    // Counter for lock duration (uses debounceInterval_)
	float lastInputValue_; // Previous input value for change detection
	float changeThreshold_; // Minimum change required to engage lock (0 = disabled)
};

