#include "debouncer.h"

void Debouncer::setup(float timeToDebounceMs, float sampleRate) {
	// in samples
	debounceInterval_ = (unsigned int)(timeToDebounceMs * sampleRate / 1000.0f);
	rt_printf("set debounceInterval_ to %u", debounceInterval_);
	debounceCounter_ = 0;
	curState_ = kStateOpen;

}

bool Debouncer::step(unsigned int input) {
	if(curState_ == kStateOpen) {
		// Button is not pressed, could be pressed anytime
		// Input: look for switch closure
		if (input == LOW)
		{
			curState_ = kStateJustClosed;

			// take action
			return true;
		}
		else {
			return false;
		}
	}
	else if(curState_ == kStateJustClosed) {
		// Button was just pressed, wait for debounce
		// Input: run counter, wait for timeout
		debounceCounter_++;
		if (debounceCounter_ >= debounceInterval_)
		{
			curState_ = kStateClosed;
			debounceCounter_ = 0;
		}

	}
	else if(curState_ == kStateClosed) {
		// Button is pressed, could be released anytime
		// Input: look for switch opening
		if (input == HIGH)
		{
			curState_ = kStateJustOpen;
		}
		
	}
	else if(curState_ == kStateJustOpen) {
		// Button was just released, wait for debounce
		// Input: run counter, wait for timeout
		debounceCounter_++;
		if (debounceCounter_ >= debounceInterval_)
		{
			curState_ = kStateOpen;
			debounceCounter_ = 0;
		}
	}
	return false;
}