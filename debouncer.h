#pragma once
#include <Bela.h>

enum DebounceState {
	kStateOpen = 0,
	kStateJustClosed,
	kStateClosed,
	kStateJustOpen 
};

class Debouncer {
	public:
    
		Debouncer() {}

		void setup(float timeToDebounceMs, float sampleRate);

		// return true if it is time to take action (button pressed down)
		bool step(unsigned int input);


		~Debouncer() {}

	private:
		DebounceState curState_;
		
		unsigned int debounceCounter_;
		unsigned int debounceInterval_;

};