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
		// return true if it is time to take action
		bool step();


		~Debouncer() {}

	private:
		DebounceState curState_;
		
		float debounceCounter_;
		unsigned int debounceInterval_;

};