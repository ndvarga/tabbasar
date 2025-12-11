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

// AnalogDebouncer.cpp: implementation of analog debouncer

#include "AnalogDebouncer.h"
#include <Bela.h>

// Constructor specifying a sample rate, thresholds, debounce interval, and change threshold
AnalogDebouncer::AnalogDebouncer(float sampleRate, float interval, float changeThreshold, float initValue)
{
	setup(sampleRate, interval, changeThreshold, initValue);
}

// Set the sample rate, interval, thresholds, and change threshold
void AnalogDebouncer::setup(float sampleRate, float interval, float changeThreshold, float initValue)
{
	debounceInterval_ = sampleRate * interval;
	currentState_ = previousState_ = kStateUnpressed;
	counter_ = 0;
	debouncedValue_ = 0.0f;
	isLocked_ = false;
	lastInputValue_ = initValue;
  rt_printf("initValue = %f\n", initValue);
	changeThreshold_ = changeThreshold;
}

// Return the debounced value given the raw analog input
float AnalogDebouncer::process(float rawInput)
{
  // Save the current state so that if it changes, the risingEdge() and
  // fallingEdge() methods can detect it
  previousState_ = currentState_;
	
  // Calculate change from last input value
  float inputChange = rawInput - lastInputValue_;
  
  // Update last input value
  lastInputValue_ = rawInput;

	
  // Run the state machine with the current input (only if not locked)
  if(currentState_ == kStateUnpressed) {
    // Input is not pressed, but look for a change in value
    // Check if change threshold is enabled and if change is sufficient
    float absChange = (inputChange < 0.0f) ? -inputChange : inputChange; // Absolute value of change

    
    // Use change threshold
    if (rawInput < (kPianoUnpressedValue_ - changeThreshold_)) {
      rt_printf("\nJust pressed\n");
      currentState_ = kStateJustPressed;
      counter_ = 0;
      isLocked_ = true;
      debouncedValue_ = rawInput;
      return debouncedValue_;
    }
    // Return 12.0f to indicate that the input is not ready
    return 12.0f;
  }

  // If the input is just pressed, wait for debounce
  else if(currentState_ == kStateJustPressed) {
    // Input was just pressed, wait for debounce
    // Run counter, wait for timeout
  
    // Still high, increment counter
    counter_++;
    if(counter_ >= debounceInterval_) {
      rt_printf("state is now pressed\n");
      
      // Timeout: now we can start waiting for the input to go low
      currentState_ = kStatePressed;
    }
    
    debouncedValue_ = rawInput;

    // Return 12.0f to indicate that the input is not ready
    return 12.0f;
  }

  else if(currentState_ == kStatePressed) {
    // If the input is pressed, return the value
    float absChange = (inputChange < 0.0f) ? -inputChange : inputChange; // Absolute value of change
    
    if (absChange >= changeThreshold_) {
      rt_printf("Just unpressed\n");
      currentState_ = kStateJustUnpressed;
      counter_ = 0;
      isLocked_ = true;
      debouncedValue_ = rawInput;
      return debouncedValue_;
    }
    else {
      isLocked_ = false;
      return 12.0f;
    }
  }

  else if(currentState_ == kStateJustUnpressed) {

    // once the lock counter is greater than the debounce interval, return the input value
    counter_++;
    if (counter_ >= debounceInterval_) {
      rt_printf("just unpressed\n");
      currentState_ = kStateUnpressed;
      isLocked_ = false;
      debouncedValue_ = rawInput;
      return debouncedValue_;
    }
    
    // else return 12.0f to indicate that the input is not ready
    return 12.0f;
  }
  

  return -2.0f;
}

// Return the current debounced value
float AnalogDebouncer::currentValue()
{
	return debouncedValue_;
}

// Return whether the input just now went pressed
bool AnalogDebouncer::justPressed()
{
	if(currentState_ == kStateJustPressed && previousState_ == kStateUnpressed)
		return true;
	return false;
}

	
// Return whether the input just now went unpressed
bool AnalogDebouncer::justUnpressed()
{
	if(currentState_ == kStateJustUnpressed && previousState_ == kStatePressed)
		return true;
	return false;	
}

// Return whether the lock is currently engaged
bool AnalogDebouncer::isLocked()
{
	return isLocked_;
}

// Return the last input value
float AnalogDebouncer::getLastInputValue()
{
	return lastInputValue_;
}

int AnalogDebouncer::getCurrentState() {
	return currentState_;
}

void AnalogDebouncer::resetState(int state) {
  currentState_ = state;
  previousState_ = state;

}

// Destructor
AnalogDebouncer::~AnalogDebouncer()
{
}

